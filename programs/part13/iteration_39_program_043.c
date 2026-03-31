/* Compile with: g++ -O2 -femulated-tls -fno-common -fvisibility=hidden -fPIC -o test_emutls test_emutls.cc */

#include <cstdio>
#include <cstdint>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak)) __attribute__((visibility("hidden"))) 
int tls_weak_hidden = 100;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 200;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 300;

/* 4. Common linkage (tentative definition) - use -fno-common to test */
__thread int tls_common;

/* 5. External declaration (defined in same file later) */
extern __thread int tls_external;

/* 6. DLL import simulation (using dllimport attribute when available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* Simulate with visibility and weak */
    __thread __attribute__((weak)) __attribute__((visibility("default"))) 
    int tls_dllimport = 600;
#endif

/* 7. Static TLS inside a namespace (C++ specific) */
namespace EmuTLS_Test {
    __thread int tls_namespace = 700;
    
    /* 8. Static TLS with internal linkage */
    static __thread int tls_static_internal = 800;
}

/* 9. Definition of external TLS variable */
__thread int tls_external = 400;

/* 10. Weak undefined TLS variable */
extern __thread __attribute__((weak)) int tls_weak_undefined;

/* ===== HELPER FUNCTIONS TO FORCE TLS USAGE ===== */

NOINLINE void use_tls_weak_hidden(volatile int* counter) {
    /* Take address and use through volatile pointer */
    volatile int* ptr = &tls_weak_hidden;
    *ptr += *counter;
    (*counter)++;
}

NOINLINE void use_tls_public_default(volatile int* counter) {
    /* Multiple accesses to ensure TREE_USED is set */
    if (tls_public_default > 0) {
        tls_public_default += *counter;
    }
    volatile int* ptr = &tls_public_default;
    *ptr += 1;
    (*counter)++;
}

NOINLINE void use_tls_protected(volatile int* counter) {
    /* Complex access pattern */
    int local = tls_protected;
    for (volatile int i = 0; i < 3; i++) {
        local += i;
    }
    tls_protected = local + *counter;
    (*counter)++;
}

NOINLINE void use_tls_common(volatile int* counter) {
    /* Force common TLS usage */
    tls_common = *counter * 2;
    volatile int* ptr = &tls_common;
    *ptr += 5;
    (*counter)++;
}

NOINLINE void use_namespace_tls(volatile int* counter) {
    using namespace EmuTLS_Test;
    
    /* Access namespace TLS */
    tls_namespace += *counter;
    
    /* Access static TLS inside namespace */
    tls_static_internal -= *counter;
    
    /* Take addresses */
    volatile int* ptr1 = &tls_namespace;
    volatile int* ptr2 = &tls_static_internal;
    *ptr1 += 1;
    *ptr2 += 1;
    
    (*counter)++;
}

NOINLINE void use_external_tls(volatile int* counter) {
    /* Use external TLS definition */
    tls_external = *counter * 10;
    
    /* Conditional based on TLS value */
    volatile int* ptr = &tls_external;
    if (*ptr > 100) {
        *ptr = 50;
    }
    
    (*counter)++;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ===== */

CONSTRUCTOR(101) static void init_tls_in_constructor() {
    /* This should force DECL_PRESERVE_P to be set */
    tls_public_default = 999;
    tls_common = 888;
    
    /* Access weak TLS */
    if (&tls_weak_hidden != nullptr) {
        tls_weak_hidden = 777;
    }
}

CONSTRUCTOR(102) static void init_namespace_tls() {
    EmuTLS_Test::tls_namespace = 1234;
    EmuTLS_Test::tls_static_internal = 5678;
}

DESTRUCTOR static void cleanup_tls() {
    /* Access TLS in destructor */
    volatile int dummy = tls_public_default;
    (void)dummy;
}

/* ===== BLOCK SCOPED TLS (tests DECL_CONTEXT) ===== */

NOINLINE void function_with_local_tls() {
    /* TLS in function scope */
    static __thread int local_func_tls = 555;
    
    /* Use it */
    local_func_tls++;
    volatile int* ptr = &local_func_tls;
    *ptr += 2;
    
    /* TLS in block scope */
    {
        __thread int block_tls = 666;
        block_tls += local_func_tls;
        volatile int* block_ptr = &block_tls;
        *block_ptr = 333;
    }
}

/* ===== MAIN EXECUTION FLOW ===== */

int main() {
    volatile int counter = 1;
    uint32_t checksum = 0;
    
    /* Force all TLS variables to be marked as used */
    use_tls_weak_hidden(&counter);
    use_tls_public_default(&counter);
    use_tls_protected(&counter);
    use_tls_common(&counter);
    use_namespace_tls(&counter);
    use_external_tls(&counter);
    function_with_local_tls();
    
    /* Conditional access based on volatile selector */
    volatile int selector = 0;
    
    for (volatile int i = 0; i < 5; i++) {
        selector = i % 4;
        
        switch (selector) {
            case 0:
                checksum += tls_weak_hidden;
                break;
            case 1:
                checksum += tls_public_default;
                break;
            case 2:
                checksum += tls_protected;
                break;
            case 3:
                checksum += tls_common;
                break;
        }
        
        /* Mix in namespace TLS */
        checksum += EmuTLS_Test::tls_namespace;
    }
    
    /* Access DLL import-like TLS */
    if (&tls_dllimport != nullptr) {
        tls_dllimport = checksum % 100;
        checksum += tls_dllimport;
    }
    
    /* Access weak undefined TLS (should be NULL) */
    if (&tls_weak_undefined != nullptr) {
        checksum += 1;
    }
    
    /* Compute final checksum using all TLS variables */
    checksum += tls_external;
    checksum += EmuTLS_Test::tls_static_internal;
    
    /* Prevent optimization */
    volatile uint32_t final_result = checksum;
    
    printf("TLS Checksum: %u\n", (unsigned int)final_result);
    
    /* Runtime verification of emulated TLS */
    printf("TLS variable addresses:\n");
    printf("  tls_weak_hidden: %p\n", (void*)&tls_weak_hidden);
    printf("  tls_public_default: %p\n", (void*)&tls_public_default);
    printf("  tls_protected: %p\n", (void*)&tls_protected);
    printf("  tls_common: %p\n", (void*)&tls_common);
    printf("  tls_external: %p\n", (void*)&tls_external);
    printf("  tls_namespace: %p\n", (void*)&EmuTLS_Test::tls_namespace);
    
    return (int)(final_result % 256);
}

/* ===== ADDITIONAL C++ SPECIFIC TESTS ===== */

#ifdef __cplusplus
namespace {
    /* Anonymous namespace TLS */
    __thread int tls_anonymous = 900;
}

class TLSContainer {
public:
    /* Static member TLS */
    static __thread int tls_static_member;
    
    NOINLINE void use_member_tls() {
        tls_static_member++;
        volatile int* ptr = &tls_static_member;
        *ptr += 2;
    }
};

/* Definition of static member TLS */
__thread int TLSContainer::tls_static_member = 1000;

/* Test function using class TLS */
NOINLINE void test_class_tls() {
    TLSContainer obj;
    obj.use_member_tls();
    
    /* Use anonymous namespace TLS */
    tls_anonymous += TLSContainer::tls_static_member;
}

/* Call from main or another constructor */
CONSTRUCTOR(103) static void init_cpp_tls() {
    test_class_tls();
}
#endif
