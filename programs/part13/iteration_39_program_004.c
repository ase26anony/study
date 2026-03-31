/* Compile with: g++ -O2 -femulated-tls -fno-common -fvisibility=hidden -o test_tls test_tls.cc */
/* For ARM targets: add -march=armv7-a or similar without native TLS support */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* 2. Hidden visibility TLS */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* 3. Protected visibility TLS */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - forces DECL_COMMON */
__thread int tls_common_var;  /* No initializer = common */

/* 5. External declaration (forces DECL_EXTERNAL) */
extern __thread int tls_external_var;

/* 6. DLL import simulation (Windows-style) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#else
/* Simulate with visibility and weak */
__thread int tls_dllimport_var __attribute__((weak, visibility("default")));
#endif

/* 7. Static TLS inside function (tests DECL_CONTEXT) */
static void function_with_static_tls() {
    static __thread int tls_function_static = 999;
    volatile int* volatile ptr = &tls_function_static;
    *ptr += 1;  /* Force usage */
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_preserved_var __attribute__((used)) = 333;

/* 9. Public TLS variable */
__thread int tls_public_var = 444;
/* Make it public explicitly */
__attribute__((used)) void* get_tls_public_addr() { return &tls_public_var; }

/* ===== C++ NAMESPACE TEST ===== */
#ifdef __cplusplus
namespace TLSNameSpace {
    __thread int tls_in_namespace = 555;
    
    class TLSUser {
    public:
        NOINLINE void modify_tls() {
            volatile int* ptr = &tls_in_namespace;
            *ptr += 10;
        }
        
        NOINLINE static void* get_tls_addr() {
            return &tls_in_namespace;
        }
    };
}
#endif

/* ===== HELPER FUNCTIONS THAT TAKE ADDRESSES ===== */

NOINLINE void use_weak_tls_addr(volatile int** out) {
    *out = &tls_weak_var;
}

NOINLINE void use_hidden_tls_addr(volatile int** out) {
    *out = &tls_hidden_var;
}

NOINLINE void use_protected_tls_addr(volatile int** out) {
    *out = &tls_protected_var;
}

NOINLINE void use_common_tls_addr(volatile int** out) {
    *out = &tls_common_var;
}

/* External TLS definition (matches earlier extern declaration) */
__thread int tls_external_var = 777;

NOINLINE void use_external_tls_addr(volatile int** out) {
    *out = &tls_external_var;
}

NOINLINE void use_dllimport_tls_addr(volatile int** out) {
    *out = &tls_dllimport_var;
}

NOINLINE void use_preserved_tls_addr(volatile int** out) {
    *out = &tls_preserved_var;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTION ===== */

CONSTRUCTOR static void init_tls_in_constructor() {
    /* This should force DECL_PRESERVE_P to be set */
    tls_preserved_var = 0xABCD;
    tls_hidden_var = 0x1234;
    
    /* Take address in constructor */
    volatile int* ptr = &tls_protected_var;
    *ptr = 0x5678;
}

DESTRUCTOR static void cleanup_tls_in_destructor() {
    /* Access TLS in destructor */
    tls_public_var = 0;
}

/* ===== MAIN TEST LOGIC ===== */

int main() {
    volatile int selector = 0;
    uintptr_t checksum = 0;
    
    /* Force all TLS variables to be marked as used */
    TREE_USED simulation: all variables have addresses taken */
    volatile int* addrs[10];
    
    /* Take addresses through non-inlineable functions */
    use_weak_tls_addr(&addrs[0]);
    use_hidden_tls_addr(&addrs[1]);
    use_protected_tls_addr(&addrs[2]);
    use_common_tls_addr(&addrs[3]);
    use_external_tls_addr(&addrs[4]);
    use_dllimport_tls_addr(&addrs[5]);
    use_preserved_tls_addr(&addrs[6]);
    
    /* Initialize common variable */
    tls_common_var = 888;
    
    /* Initialize DLL import simulation variable */
    tls_dllimport_var = 999;
    
    /* Call function with static TLS */
    function_with_static_tls();
    
#ifdef __cplusplus
    /* Use namespace TLS variable */
    TLSNameSpace::TLSUser user;
    user.modify_tls();
    addrs[7] = (volatile int*)TLSNameSpace::TLSUser::get_tls_addr();
#endif
    
    /* Force TREE_USED on public variable by taking address */
    addrs[8] = &tls_public_var;
    
    /* Volatile loop with conditional TLS access */
    for (volatile int i = 0; i < 10; i++) {
        selector = i % 9;
        volatile int* ptr = addrs[selector];
        
        /* Modify through volatile pointer to prevent optimization */
        if (ptr) {
            *ptr += (i + 1);
        }
    }
    
    /* Compute checksum of all TLS variables */
    for (int i = 0; i < 9; i++) {
        if (addrs[i]) {
            checksum += (uintptr_t)addrs[i];  /* Address checksum */
            checksum += *addrs[i];            /* Value checksum */
        }
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    
    /* Verify emulated TLS structure by checking addresses */
    printf("TLS variable addresses:\n");
    printf("  weak: %p\n", &tls_weak_var);
    printf("  hidden: %p\n", &tls_hidden_var);
    printf("  protected: %p\n", &tls_protected_var);
    printf("  common: %p\n", &tls_common_var);
    printf("  external: %p\n", &tls_external_var);
    printf("  dllimport: %p\n", &tls_dllimport_var);
    printf("  preserved: %p\n", &tls_preserved_var);
    printf("  public: %p\n", &tls_public_var);
    
#ifdef __cplusplus
    printf("  namespace: %p\n", &TLSNameSpace::tls_in_namespace);
#endif
    
    return 0;
}

/* Additional external reference to force DECL_EXTERNAL handling */
extern __thread int tls_unused_external;
void* dummy_ref = &tls_unused_external;  /* Take address to force reference */
