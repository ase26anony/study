/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS by using non-TLS target flags */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

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
    tls_function_local++;  /* Ensure TREE_USED */
}

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* Ensure all TLS variables are marked TREE_USED */
void reference_all_tls(void) {
    /* Reference each variable to ensure TREE_USED is set */
    tls_used++;
    tls_public++;
    tls_static++;
    tls_common = 1;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    
    /* Reference external TLS */
    if (tls_external > 0) {
        tls_external++;
    }
    
    function_with_tls();
}

/* Main entry point for C test */
int main_c(void) {
    reference_all_tls();
    return 0;
}
