/* Test TLS variables with various attributes to trigger EMUTLS attribute copying */
#include <stdio.h>

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS inside function scope */
void function_with_tls(void) {
    __thread int tls_function_local = 600;
    tls_function_local++;  /* TREE_USED */
}

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Reference all TLS variables to ensure TREE_USED is set */
void use_all_tls_variables(void) {
    /* Read and write to each TLS variable */
    tls_used += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common = 123;
    tls_weak += 4;
    tls_hidden += 5;
    tls_default += 6;
    
    /* Use external TLS */
    tls_external += 7;
    
    /* Call function with local TLS */
    function_with_tls();
}

/* Main entry point */
int main(void) {
    use_all_tls_variables();
    
    /* Print some values to prevent optimization */
    printf("TLS values: %d %d %d\n", tls_used, tls_public, tls_common);
    
    return 0;
}
