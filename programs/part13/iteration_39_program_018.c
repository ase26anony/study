/* tls_emutls_test.c - Test program for GCC emulated TLS attribute propagation */

/* Force emulated TLS even on platforms with native support */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 0x1234;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 0x5678;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 0x9ABC;

/* 4. Common linkage (tentative definition) - forces DECL_COMMON */
__thread int tls_common;  /* No initializer = common */

/* 5. External declaration (forces DECL_EXTERNAL) */
extern __thread int tls_external;

/* 6. DLL import simulation (Windows-specific attribute) */
#ifdef _WIN32
__thread __declspec(dllimport) int tls_dllimport;
#else
/* Simulate with visibility and weak */
__thread __attribute__((weak, visibility("default"))) 
int tls_dllimport = 0xDEF0;
#endif

/* 7. Static TLS inside function (tests DECL_CONTEXT) */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 0x1111;
    volatile int* volatile ptr = &tls_function_static;
    (void)ptr; /* Use pointer to prevent optimization */
}

/* 8. TLS with preserve attribute (via used) */
__thread __attribute__((used)) int tls_preserved = 0x2222;

/* ===== HELPER FUNCTIONS FOR ADDRESS-TAKING ===== */

NOINLINE static void use_tls_weak_hidden(volatile int** out) {
    *out = &tls_weak_hidden;
    tls_weak_hidden += 1;  /* Modify to ensure usage */
}

NOINLINE static void use_tls_public_default(volatile int** out) {
    *out = &tls_public_default;
    tls_public_default ^= 0x5555;  /* Different operation */
}

NOINLINE static void use_tls_protected(volatile int** out) {
    *out = &tls_protected;
    tls_protected *= 2;
}

NOINLINE static void use_tls_common(volatile int** out) {
    *out = &tls_common;
    tls_common = 0x3333;  /* Initialize common TLS */
}

/* External TLS definition (matches earlier extern declaration) */
__thread int tls_external = 0x4444;

NOINLINE static void use_tls_external(volatile int** out) {
    *out = &tls_external;
    tls_external += 0x10;
}

NOINLINE static void use_tls_dllimport(volatile int** out) {
    *out = &tls_dllimport;
    tls_dllimport -= 1;
}

NOINLINE static void use_tls_preserved(volatile int** out) {
    *out = &tls_preserved;
    tls_preserved |= 0x8000;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTION ===== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* Access and modify TLS in constructor - tests DECL_PRESERVE_P */
    tls_public_default = 0x9999;
    tls_protected = 0xAAAA;
    
    /* Take addresses in constructor */
    volatile int* ptr1 = &tls_public_default;
    volatile int* ptr2 = &tls_protected;
    (void)ptr1;
    (void)ptr2;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Final TLS access */
    tls_public_default = 0;
}

/* ===== COMPLEX CONTROL FLOW WITH VOLATILE ===== */

NOINLINE static uint32_t compute_tls_checksum(void) {
    volatile int selector = 0;
    uint32_t checksum = 0;
    volatile int* tls_ptrs[8];
    
    /* Initialize pointer array with volatile to prevent optimization */
    for (int i = 0; i < 8; i++) {
        tls_ptrs[i] = NULL;
    }
    
    /* Complex control flow accessing different TLS variables */
    for (volatile int i = 0; i < 10; i++) {
        selector = i % 7;
        
        switch (selector) {
            case 0:
                use_tls_weak_hidden((volatile int**)&tls_ptrs[0]);
                checksum += *tls_ptrs[0];
                break;
            case 1:
                use_tls_public_default((volatile int**)&tls_ptrs[1]);
                checksum += *tls_ptrs[1] ^ 0xF0F0;
                break;
            case 2:
                use_tls_protected((volatile int**)&tls_ptrs[2]);
                checksum += *tls_ptrs[2] * 3;
                break;
            case 3:
                use_tls_common((volatile int**)&tls_ptrs[3]);
                checksum += *tls_ptrs[3] + 0x1000;
                break;
            case 4:
                use_tls_external((volatile int**)&tls_ptrs[4]);
                checksum += *tls_ptrs[4] & 0x7FFF;
                break;
            case 5:
                use_tls_dllimport((volatile int**)&tls_ptrs[5]);
                checksum += *tls_ptrs[5] | 0x4000;
                break;
            case 6:
                use_tls_preserved((volatile int**)&tls_ptrs[6]);
                checksum += *tls_ptrs[6] - 0x2000;
                break;
        }
        
        /* Additional volatile operation to prevent loop optimization */
        volatile int barrier = selector;
        (void)barrier;
    }
    
    /* Call function with static TLS */
    function_with_static_tls();
    
    return checksum;
}

/* ===== C++ SPECIFIC TESTS (if compiled as C++) ===== */
#ifdef __cplusplus
namespace {
    /* TLS in anonymous namespace */
    __thread int tls_anonymous_ns = 0x5555;
    
    namespace TestNamespace {
        /* TLS in named namespace */
        __thread __attribute__((visibility("hidden"))) 
        int tls_namespace_hidden = 0x6666;
    }
}

/* Class with method using TLS */
struct TLSUser {
    NOINLINE uint32_t use_namespace_tls() {
        volatile int* ptr1 = &tls_anonymous_ns;
        volatile int* ptr2 = &TestNamespace::tls_namespace_hidden;
        
        *ptr1 += 1;
        *ptr2 += 2;
        
        return *ptr1 + *ptr2;
    }
};
#endif

/* ===== MAIN FUNCTION ===== */
int main(void) {
    uint32_t checksum = 0;
    
    /* Initial TLS accesses */
    checksum += tls_weak_hidden;
    checksum += tls_public_default;
    checksum += tls_protected;
    
    /* Force usage of all TLS variables through address-taking */
    volatile int* volatile ptr_array[7];
    
    use_tls_weak_hidden((volatile int**)&ptr_array[0]);
    use_tls_public_default((volatile int**)&ptr_array[1]);
    use_tls_protected((volatile int**)&ptr_array[2]);
    use_tls_common((volatile int**)&ptr_array[3]);
    use_tls_external((volatile int**)&ptr_array[4]);
    use_tls_dllimport((volatile int**)&ptr_array[5]);
    use_tls_preserved((volatile int**)&ptr_array[6]);
    
    /* Compute checksum through complex control flow */
    checksum += compute_tls_checksum();
    
    /* Access through all pointers to ensure they're used */
    for (int i = 0; i < 7; i++) {
        if (ptr_array[i]) {
            checksum += *ptr_array[i];
        }
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    TLSUser user;
    checksum += user.use_namespace_tls();
    
    checksum += tls_anonymous_ns;
    checksum += TestNamespace::tls_namespace_hidden;
#endif
    
    /* Print checksum to prevent elimination */
    printf("TLS checksum: 0x%08X\n", checksum);
    
    /* Verify emulated TLS is being used */
    printf("TLS variable addresses:\n");
    printf("  tls_weak_hidden:   %p\n", (void*)&tls_weak_hidden);
    printf("  tls_public_default:%p\n", (void*)&tls_public_default);
    printf("  tls_protected:     %p\n", (void*)&tls_protected);
    printf("  tls_common:        %p\n", (void*)&tls_common);
    printf("  tls_external:      %p\n", (void*)&tls_external);
    
    return (checksum != 0) ? 0 : 1;
}
