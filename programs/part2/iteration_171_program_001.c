/* Test for TLS emulation attribute copying coverage */
/* This file defines various TLS variables with different attributes */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with used attribute - tests DECL_PRESERVE_P, TREE_USED */
__thread int tls_used __attribute__((used)) = 42;

/* Test 2: Public TLS variable - tests TREE_PUBLIC */
__thread int tls_public = 100;

/* Test 3: Weak TLS variable - tests DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 200;

/* Test 4: Hidden visibility - tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* Test 5: Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* Test 6: Common linkage (tentative definition) - tests DECL_COMMON */
__thread int tls_common;

/* Test 7: External declaration (defined in aux file) - tests DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 8: Static TLS (non-public) for contrast */
static __thread int tls_static = 500;

/* Test 9: DLL import simulation (using weak attribute as proxy) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* On non-Windows, use weak attribute to simulate similar behavior */
__thread int tls_dllimport __attribute__((weak));
#endif

/* Function to ensure TLS variables are used */
void use_tls_variables(void) {
    /* Take addresses to prevent optimization */
    volatile int *ptr;
    
    ptr = &tls_used;
    tls_used += 1;
    
    ptr = &tls_public;
    tls_public += 2;
    
    ptr = &tls_weak;
    if (&tls_weak != NULL) {
        tls_weak += 3;
    }
    
    ptr = &tls_hidden;
    tls_hidden += 4;
    
    ptr = &tls_protected;
    tls_protected += 5;
    
    ptr = &tls_common;
    tls_common = 600;
    
    ptr = &tls_static;
    tls_static += 7;
    
    /* External variable - will be defined in aux file */
    if (&tls_external != NULL) {
        tls_external += 8;
    }
    
    /* Use inline asm to ensure variables aren't optimized out */
    __asm__ volatile ("" : : "r"(&tls_used));
    __asm__ volatile ("" : : "r"(&tls_public));
    __asm__ volatile ("" : : "r"(&tls_hidden));
    __asm__ volatile ("" : : "r"(&tls_protected));
}

/* Function with local TLS variable - tests DECL_CONTEXT */
static void function_with_local_tls(void) {
    /* Local TLS variable - should have function as DECL_CONTEXT */
    static __thread int local_tls_in_func = 700;
    local_tls_in_func += 1;
    __asm__ volatile ("" : : "r"(&local_tls_in_func));
}

/* Global function that uses local TLS */
void call_local_tls_function(void) {
    function_with_local_tls();
}

/* Main function */
int main(void) {
    use_tls_variables();
    call_local_tls_function();
    
    /* Also test taking address of external variable */
    int *ext_ptr = &tls_external;
    (void)ext_ptr;
    
    /* Test DLL import simulation */
    if (&tls_dllimport != NULL) {
        tls_dllimport = 800;
    }
    
    return 0;
}

#ifdef __cplusplus
}
#endif
