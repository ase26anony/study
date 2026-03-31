/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS by targeting architecture without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside a function (block scope) */
void function_with_tls(void) {
    /* Block-scoped TLS - different DECL_CONTEXT */
    static __thread int tls_in_function = 600;
    tls_in_function++;
}

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* Reference all TLS variables to ensure TREE_USED is set */
void use_all_tls_variables(void) {
    /* Read and write to mark as used */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 1;
    tls_common = 999;
    tls_weak = tls_weak / 2;
    tls_hidden = tls_hidden + 100;
    tls_default = tls_default - 50;
    
    /* Use the external variable */
    tls_external = tls_external + 1000;
    
    /* Call function to use its TLS */
    function_with_tls();
}

/* Main function for C test */
int main_c(void) {
    use_all_tls_variables();
    
    /* Additional uses to ensure coverage */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    
    function_with_tls();
    
    return sum != 0 ? 0 : 1;  /* Ensure all variables contribute */
}
