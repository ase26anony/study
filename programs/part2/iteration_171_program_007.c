/* Test for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__thread int tls_used __attribute__((used)) = 42;

/* Test 2: TLS with hidden visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Test 3: TLS with protected visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* Test 4: External TLS declaration (will be defined in aux file) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 5: Common TLS (tentative definition) */
/* Tests: DECL_COMMON, TREE_PUBLIC */
__thread int tls_common;

/* Test 6: Weak TLS symbol */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__thread int tls_weak __attribute__((weak)) = 300;

/* Test 7: Static TLS (non-public) inside function scope */
/* Tests: DECL_CONTEXT (when inside function), !TREE_PUBLIC */
static void test_function_scope(void) {
    static __thread int tls_static_func = 500;
    (void)tls_static_func; /* Prevent unused warning */
}

/* Test 8: TLS with internal visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* Forward declaration for function in aux file */
void use_tls_variables(void);

/* Main function that uses all TLS variables to prevent optimization */
int main(void) {
    /* Take addresses to ensure variables are processed */
    int *ptrs[] = {
        &tls_used,
        &tls_hidden,
        &tls_protected,
        &tls_external,
        &tls_common,
        &tls_weak,
        &tls_internal,
        NULL
    };
    
    /* Modify some variables */
    tls_used += 1;
    tls_hidden *= 2;
    tls_protected -= 10;
    tls_common = 999;
    
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(&tls_used));
    asm volatile("" : : "r"(&tls_hidden));
    asm volatile("" : : "r"(&tls_protected));
    asm volatile("" : : "r"(&tls_external));
    asm volatile("" : : "r"(&tls_weak));
    asm volatile("" : : "r"(&tls_internal));
    
    /* Call function from other compilation unit */
    use_tls_variables();
    
    /* Access function-scoped TLS */
    test_function_scope();
    
    return 0;
}

#ifdef __cplusplus
}
#endif
