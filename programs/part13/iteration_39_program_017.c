/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even on platforms with native support */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden")));

/* 2. Public TLS with default visibility (explicit) */
__thread int tls_public_default __attribute__((visibility("default")));
TREE_PUBLIC should be set

/* 3. Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected")));

/* 4. Common linkage TLS (tentative definition) */
__thread int tls_common;  /* Should get DECL_COMMON */

/* 5. External TLS declaration (DECL_EXTERNAL should be true) */
extern __thread int tls_external;

/* 6. Static TLS within function context - tests DECL_CONTEXT */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 42;
    (void)tls_function_static;
}

/* 7. DLL import simulation (for Windows targets) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dll_import;
#else
    /* Simulate with attribute on non-Windows */
    __thread int tls_dll_import __attribute__((dllimport));
#endif

/* 8. Preserve flag test - used in constructor */
__thread int tls_preserve_flag;

/* 9. Weak undefined TLS */
extern __thread int tls_weak_undefined __attribute__((weak));

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_address_weak_hidden(int **ptr) {
    *ptr = &tls_weak_hidden;
    TREE_USED should be marked
}

NOINLINE static void use_tls_address_public(int **ptr) {
    *ptr = &tls_public_default;
}

NOINLINE static void use_tls_address_protected(int **ptr) {
    *ptr = &tls_protected;
}

NOINLINE static void use_tls_address_common(int **ptr) {
    *ptr = &tls_common;
}

NOINLINE static void use_tls_address_external(int **ptr) {
    /* External declaration - address might be resolved later */
    *ptr = &tls_external;
}

/* ========== VOLATILE ACCESS PATTERNS ========== */

NOINLINE static int volatile_access_tls(volatile int *counter) {
    int sum = 0;
    
    /* Force multiple TLS accesses through volatile pointer */
    for (volatile int i = 0; i < 3; i++) {
        sum += tls_weak_hidden;
        sum += tls_public_default;
        sum += tls_protected;
        (*counter)++;
    }
    
    return sum;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTION ========== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should trigger DECL_PRESERVE_P being set */
    tls_preserve_flag = 0xDEADBEEF;
    
    /* Also initialize other TLS variables */
    tls_weak_hidden = 1;
    tls_public_default = 2;
    tls_protected = 3;
    tls_common = 4;
    
    /* External definition (if linked) */
    tls_external = 5;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Access TLS in destructor */
    volatile int dummy = tls_preserve_flag;
    (void)dummy;
}

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    volatile int selector = 0;
    int *tls_pointers[5];
    int checksum = 0;
    
    /* 1. Take addresses of TLS variables - forces TREE_USED */
    use_tls_address_weak_hidden(&tls_pointers[0]);
    use_tls_address_public(&tls_pointers[1]);
    use_tls_address_protected(&tls_pointers[2]);
    use_tls_address_common(&tls_pointers[3]);
    use_tls_address_external(&tls_pointers[4]);
    
    /* 2. Volatile access pattern with conditional flow */
    for (volatile int i = 0; i < 5; i++) {
        selector = i;
        
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
            case 4:
                checksum += tls_external;
                break;
        }
        
        /* More volatile access */
        volatile_access_tls(&selector);
    }
    
    /* 3. Compute final checksum through pointers */
    for (int i = 0; i < 5; i++) {
        if (tls_pointers[i]) {
            checksum += *tls_pointers[i];
        }
    }
    
    /* 4. Call function with static TLS to test DECL_CONTEXT */
    function_with_static_tls();
    
    /* 5. Use DLL import TLS if available */
    checksum += tls_dll_import;
    
    /* 6. Use weak undefined TLS */
    checksum += tls_weak_undefined;
    
    printf("TLS checksum: %d\n", checksum);
    printf("tls_preserve_flag: 0x%x\n", tls_preserve_flag);
    
    return checksum != 0 ? 0 : 1;
}

/* ========== EXTERNAL TLS DEFINITION (in same file for testing) ========== */
__thread int tls_external = 100;

/* Weak undefined remains undefined - tests weak attribute propagation */
/* __thread int tls_weak_undefined; */ /* Uncomment to provide definition */

/* ========== C++ SPECIFIC TESTS (compile as C++) ========== */
#ifdef __cplusplus
namespace tls_test {
    __thread int tls_in_namespace = 123;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            /* Access TLS from namespace */
            volatile int* ptr = &tls_in_namespace;
            *ptr += 1;
        }
    };
}

/* Additional C++ test */
static void test_cpp_tls(void) {
    tls_test::TLSUser user;
    user.use_namespace_tls();
}
#endif
