/* Test program for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Public TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */
__attribute__((used))
__thread int tls_public_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK */
__attribute__((weak))
__thread int tls_weak;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;

/* Test 4: Protected visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("protected")))
__thread int tls_protected;

/* Test 5: External declaration (will be defined in aux file) */
/* Tests: DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 6: Common TLS (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common;

/* Test 7: DLL import style (using weak alias for emulation) */
/* Tests: DECL_DLLIMPORT_P (via weak attribute) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, simulate with weak external */
extern __thread int tls_imported __attribute__((weak));
#endif

/* Test 8: Static TLS with internal linkage */
/* Tests: TREE_PUBLIC = 0, has DECL_CONTEXT */
static __thread int tls_static_internal = 7;

/* Test 9: TLS in function scope for DECL_CONTEXT */
static void test_function_scope(void) {
    /* Local TLS variable - will have function as DECL_CONTEXT */
    static __thread int tls_function_local = 99;
    (void)tls_function_local; /* Use it */
}

/* Forward declaration for function in aux file */
void use_tls_variables(void);

/* Helper to prevent optimization */
static void touch_variable(void *ptr) {
    __asm__ volatile ("" : : "r"(ptr) : "memory");
}

int main(void) {
    /* Take addresses of all TLS variables to ensure they're processed */
    touch_variable(&tls_public_used);
    touch_variable(&tls_weak);
    touch_variable(&tls_hidden);
    touch_variable(&tls_protected);
    touch_variable(&tls_external);
    touch_variable(&tls_common);
    touch_variable(&tls_imported);
    touch_variable(&tls_static_internal);
    
    /* Modify some variables to ensure they're used */
    tls_public_used += 1;
    tls_hidden *= 2;
    tls_static_internal -= 3;
    
    /* Access weak variable */
    if (&tls_weak != NULL) {
        tls_weak = 123;
    }
    
    /* Call function that uses external TLS */
    use_tls_variables();
    
    /* Test function scope TLS */
    test_function_scope();
    
    /* Simple validation */
    int sum = tls_public_used + tls_hidden + tls_static_internal;
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(sum));
    
    return 0;
}

#ifdef __cplusplus
}
#endif
