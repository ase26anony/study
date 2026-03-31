/* Test file for EMUTLS attribute copying - C definitions */

/* Force EMUTLS by targeting ARM without hardware TLS support */
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

/* DECL_CONTEXT: TLS variable inside a function */
void set_function_tls(void) {
    __thread int tls_function_local = 600;
    tls_function_local = 601;  /* TREE_USED: reference it */
}

/* External declaration (will be defined in another file) */
extern __thread int tls_external;

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_vars(void) {
    /* TREE_USED: reference each variable */
    int val;
    
    val = tls_used;
    tls_used = val + 1;
    
    val = tls_public;
    tls_public = val + 1;
    
    val = tls_static;
    tls_static = val + 1;
    
    val = tls_common;
    tls_common = val + 1;
    
    val = tls_weak;
    tls_weak = val + 1;
    
    val = tls_hidden;
    tls_hidden = val + 1;
    
    val = tls_default;
    tls_default = val + 1;
    
    val = tls_external;
    tls_external = val + 1;
    
    set_function_tls();
}
