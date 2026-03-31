/* Test for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with used attribute (tests DECL_PRESERVE_P) */
__thread int tls_used_var __attribute__((used)) = 42;

/* Test 2: Public TLS with default visibility (tests TREE_PUBLIC, DECL_VISIBILITY) */
__thread int tls_public_var __attribute__((visibility("default"))) = 100;

/* Test 3: Weak TLS variable (tests DECL_WEAK) */
__thread int tls_weak_var __attribute__((weak)) = 200;

/* Test 4: Hidden visibility TLS (tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED) */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 300;

/* Test 5: Protected visibility TLS */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 400;

/* Test 6: Static TLS (non-public, tests contrast for TREE_PUBLIC) */
static __thread int tls_static_var = 500;

/* Test 7: External declaration (tests DECL_EXTERNAL) */
extern __thread int tls_external_var;

/* Test 8: Common TLS (tests DECL_COMMON) - tentative definition */
__thread int tls_common_var;

/* Function to use TLS variables */
static void use_tls_variables(void) {
    /* Take addresses to prevent optimization */
    volatile int *ptr1 = &tls_used_var;
    volatile int *ptr2 = &tls_public_var;
    volatile int *ptr3 = &tls_weak_var;
    volatile int *ptr4 = &tls_hidden_var;
    volatile int *ptr5 = &tls_protected_var;
    volatile int *ptr6 = &tls_static_var;
    volatile int *ptr7 = &tls_external_var;
    volatile int *ptr8 = &tls_common_var;
    
    /* Modify some variables */
    tls_used_var += 1;
    tls_public_var += 2;
    tls_common_var = 600;
    
    /* Use inline asm to ensure variables are referenced */
    __asm__ volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    __asm__ volatile ("" : : "r"(ptr4), "r"(ptr5), "r"(ptr6));
    __asm__ volatile ("" : : "r"(ptr7), "r"(ptr8));
}

/* Function with local TLS (tests DECL_CONTEXT for function scope) */
static void function_with_local_tls(void) {
    /* Local TLS variable inside function scope */
    static __thread int local_tls_in_func = 700;
    local_tls_in_func += 1;
    __asm__ volatile ("" : : "r"(&local_tls_in_func));
}

/* Main function */
int main(void) {
    use_tls_variables();
    function_with_local_tls();
    
    /* Reference all TLS variables to ensure they're used */
    int sum = tls_used_var + tls_public_var + tls_weak_var +
              tls_hidden_var + tls_protected_var + tls_static_var +
              tls_external_var + tls_common_var;
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(sum));
    
    return 0;
}

#ifdef __cplusplus
}
#endif
