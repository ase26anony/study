/* Test for EMUTLS attribute copying - Main file */
#include <stdio.h>

/* Force EMUTLS by targeting ARM without hardware TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

/* 1. DECL_PRESERVE_P: TLS variable with 'used' attribute */
__thread int tls_used __attribute__((used)) = 42;

/* 2. TREE_PUBLIC: Non-static (public) TLS variable */
__thread int tls_public = 100;

/* 3. TREE_PUBLIC: Static (non-public) TLS variable */
static __thread int tls_static = 200;

/* 4. DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;  /* No initializer */

/* 5. DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* 6. DECL_VISIBILITY: TLS with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* 7. DECL_VISIBILITY: TLS with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* 8. DECL_EXTERNAL: External declaration (defined in another file) */
extern __thread int tls_external;

/* Function to use all TLS variables */
void use_tls_variables(void) {
    /* TREE_USED: Reference all variables to mark them as used */
    tls_used += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common = tls_public + tls_static;
    tls_weak += 4;
    tls_hidden += 5;
    tls_default += 6;
    
    /* Use external TLS variable */
    tls_external += 7;
    
    printf("TLS values: %d, %d, %d, %d, %d, %d, %d, %d\n",
           tls_used, tls_public, tls_static, tls_common,
           tls_weak, tls_hidden, tls_default, tls_external);
}

/* Function with local TLS variable (different DECL_CONTEXT) */
void function_with_local_tls(void) {
    /* DECL_CONTEXT: TLS variable inside a function */
    static __thread int local_func_tls = 600;
    local_func_tls++;
    printf("Local func TLS: %d\n", local_func_tls);
}

int main(void) {
    /* Use all TLS variables multiple times */
    for (int i = 0; i < 3; i++) {
        use_tls_variables();
        function_with_local_tls();
    }
    
    return 0;
}
