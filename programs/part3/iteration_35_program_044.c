/* Test TLS variables with various attributes to trigger EMUTLS attribute copying */

/* Force EMUTLS by using non-TLS-supporting target flags in compilation */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable with initializer */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED: explicit visibility attributes */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside function scope */
void function_with_tls(void) {
    __thread int tls_function_scope = 600;
    tls_function_scope++;  /* Ensure TREE_USED */
}

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_variables(void) {
    /* Read and write to each TLS variable */
    int sum = 0;
    
    sum += tls_used;
    tls_used = sum;
    
    sum += tls_public;
    tls_public = sum;
    
    sum += tls_static;
    tls_static = sum;
    
    tls_common = sum;
    sum += tls_common;
    
    sum += tls_weak;
    tls_weak = sum;
    
    sum += tls_hidden;
    tls_hidden = sum;
    
    sum += tls_default;
    tls_default = sum;
    
    sum += tls_external;
    
    function_with_tls();
}

/* Main function for C test */
int main_c(void) {
    reference_tls_variables();
    return 0;
}
