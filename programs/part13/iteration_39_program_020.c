/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even on platforms with native support */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 0x1234;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 0x5678;

/* 3. External TLS declaration (defined in another TU) */
extern __thread int tls_external;

/* 4. Common linkage TLS (tentative definition) */
__thread int tls_common;

/* 5. Protected visibility TLS */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 0x9ABC;

/* 6. DLL import simulation (Windows-style) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with weak external */
extern __thread __attribute__((weak)) int tls_dllimport;
#endif

/* 7. Static TLS inside function context */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 0xDEF0;
    (void)tls_function_static;
}

/* 8. TLS with used attribute (force emission) */
__thread __attribute__((used)) int tls_used = 0x1111;

/* ========== C++ SPECIFIC TESTS (if compiled as C++) ========== */
#ifdef __cplusplus
namespace TLSNameSpace {
    /* 9. TLS in namespace with internal linkage */
    static __thread int tls_namespace_internal = 0x2222;
    
    /* 10. Public TLS in namespace */
    __thread int tls_namespace_public = 0x3333;
    
    class TLSClass {
    public:
        /* 11. Static TLS member */
        static __thread int tls_class_member;
        
        void access_tls() {
            /* Force usage through member function */
            volatile int* ptr = &tls_class_member;
            *ptr += 1;
        }
    };
    
    __thread int TLSClass::tls_class_member = 0x4444;
}
#endif

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_weak_hidden(volatile int* counter) {
    volatile int* ptr = &tls_weak_hidden;
    *ptr += *counter;
    *counter += 1;
}

NOINLINE static void use_tls_public_default(volatile int* counter) {
    volatile int* ptr = &tls_public_default;
    *ptr ^= *counter;  /* Different operation to avoid pattern recognition */
    *counter += 2;
}

NOINLINE static void use_tls_external(volatile int* counter) {
    /* External TLS - compiler must generate proper references */
    volatile int* ptr = &tls_external;
    if (ptr) {
        *counter += 3;
    }
}

NOINLINE static void use_tls_common(volatile int* counter) {
    volatile int* ptr = &tls_common;
    *ptr |= *counter;
    *counter += 4;
}

NOINLINE static void use_tls_protected(volatile int* counter) {
    volatile int* ptr = &tls_protected;
    *ptr &= ~(*counter);
    *counter += 5;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void init_tls_values(void) {
    /* Access and modify TLS in constructor - tests DECL_PRESERVE_P */
    tls_public_default = 0xCAFE;
    tls_protected = 0xBABE;
    
#ifdef __cplusplus
    TLSNameSpace::tls_namespace_public = 0xDEAD;
    TLSNameSpace::TLSClass::tls_class_member = 0xBEEF;
#endif
}

DESTRUCTOR static void cleanup_tls_values(void) {
    /* Final TLS access in destructor */
    volatile int dummy = tls_public_default;
    (void)dummy;
}

/* ========== COMPLEX CONTROL FLOW ========== */

NOINLINE static uint32_t compute_tls_checksum(volatile int selector) {
    uint32_t checksum = 0;
    volatile int* tls_pointers[6];
    int i;
    
    /* Take addresses of TLS variables - forces TREE_USED marking */
    tls_pointers[0] = &tls_weak_hidden;
    tls_pointers[1] = &tls_public_default;
    tls_pointers[2] = &tls_external;
    tls_pointers[3] = &tls_common;
    tls_pointers[4] = &tls_protected;
    tls_pointers[5] = &tls_used;
    
    /* Complex control flow based on volatile selector */
    for (i = 0; i < 6; i++) {
        if ((selector >> i) & 1) {
            checksum += (uintptr_t)tls_pointers[i];
            checksum ^= *tls_pointers[i];
        } else {
            checksum += i * 0x1000;
        }
        
        /* Volatile memory access pattern */
        volatile int temp = selector;
        selector = temp + 1;
    }
    
    return checksum;
}

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    volatile int counter = 0;
    volatile int selector = 0x3F;  /* Start with all bits set */
    uint32_t final_checksum = 0;
    
    /* 1. Force usage of all TLS variables through address-taking */
    use_tls_weak_hidden(&counter);
    use_tls_public_default(&counter);
    use_tls_external(&counter);
    use_tls_common(&counter);
    use_tls_protected(&counter);
    
    /* 2. Call function with static TLS */
    function_with_static_tls();
    
#ifdef __cplusplus
    /* 3. C++ specific TLS usage */
    {
        TLSNameSpace::TLSClass obj;
        obj.access_tls();
        
        volatile int* ns_ptr = &TLSNameSpace::tls_namespace_internal;
        *ns_ptr += counter;
    }
#endif
    
    /* 4. Complex control flow with volatile accesses */
    for (int i = 0; i < 10; i++) {
        final_checksum ^= compute_tls_checksum(selector);
        
        /* Modify selector in volatile way */
        volatile int* sel_ptr = &selector;
        *sel_ptr = (*sel_ptr << 1) | ((*sel_ptr >> 5) & 1);
    }
    
    /* 5. Prevent optimization of TLS variables */
    printf("TLS checksum: 0x%08X\n", final_checksum);
    printf("Counter: %d\n", counter);
    
    /* 6. Runtime verification of emulated TLS */
    printf("TLS variable addresses:\n");
    printf("  weak_hidden:   %p\n", (void*)&tls_weak_hidden);
    printf("  public_default:%p\n", (void*)&tls_public_default);
    printf("  protected:     %p\n", (void*)&tls_protected);
    
    return (final_checksum != 0) ? 0 : 1;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This would normally be in a separate compilation unit */
__thread int tls_external = 0x5555;

/* ========== ADDITIONAL TEST FOR BLOCK SCOPE TLS ========== */
void test_block_scope_tls(void) {
    /* TLS in block scope - tests DECL_CONTEXT handling */
    static __thread int tls_block_static = 0x6666;
    volatile int* block_ptr = &tls_block_static;
    *block_ptr += 1;
    
    /* Call it once from main */
    (void)block_ptr;
}
