/* Test for EMUTLS attribute copying - Main file */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden -fPIC */

/* Force EMUTLS transformation by targeting ARM without hardware TLS support */

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

/* DECL_VISIBILITY_SPECIFIED: explicit visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* Function-scoped TLS variable (different DECL_CONTEXT) */
void use_function_tls(void) {
    /* DECL_CONTEXT: function-local TLS */
    static __thread int tls_function_local = 600;
    tls_function_local++;
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read/write operations to mark as TREE_USED */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 1;
    tls_common = 999;
    tls_weak = tls_weak / 2;
    tls_hidden = tls_hidden + 100;
    tls_default = tls_default - 50;
    
    /* Use external TLS */
    tls_external = tls_external + 10;
    
    /* Call function with local TLS */
    use_function_tls();
}

/* Main entry point */
int main(void) {
    reference_all_tls();
    
    /* Additional references to ensure variables are marked used */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_external;
    
    return sum > 0 ? 0 : 1;
}
