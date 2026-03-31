/* Test for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by targeting ARM without hardware TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

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
__thread int tls_default_vis __attribute__((visibility("default"))) = 500;

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* Function-scoped TLS variable (different DECL_CONTEXT) */
void use_function_tls(void) {
    /* DECL_CONTEXT: function scope */
    static __thread int tls_function_scope = 600;
    tls_function_scope++;
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_variables(void) {
    /* TREE_USED: reference each variable */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 1;
    tls_common = 999;
    tls_weak = tls_weak / 2;
    tls_hidden = tls_hidden + 100;
    tls_default_vis = tls_default_vis - 50;
    tls_external = tls_external + 1000;
    
    use_function_tls();
}

/* Main function for C test */
int main_c(void) {
    reference_tls_variables();
    return 0;
}
