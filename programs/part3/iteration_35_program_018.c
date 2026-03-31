/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by targeting architecture without native TLS */
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

/* DECL_DLLIMPORT_P: Windows dllimport (use appropriate attribute for target) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with appropriate attribute if available */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* Function-scoped TLS (tests DECL_CONTEXT) */
void use_function_tls(void) {
    /* DECL_CONTEXT: TLS inside function scope */
    static __thread int tls_function_scope = 600;
    tls_function_scope++;
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_variables(void) {
    /* Read/write operations to mark as used */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 1;
    tls_common = 999;
    tls_weak = tls_weak / 2;
    tls_hidden = tls_hidden + 100;
    tls_default = tls_default - 50;
    tls_dllimport = 777;
    
    /* Use external TLS */
    tls_external = tls_external + 10;
    
    /* Call function with function-scoped TLS */
    use_function_tls();
}

/* Main entry point */
int main(void) {
    reference_tls_variables();
    return 0;
}
