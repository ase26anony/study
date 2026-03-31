/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS by using non-TLS-supporting target flags */
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
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_DLLIMPORT_P: Windows dllimport (use appropriate attribute for target) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with visibility on non-Windows */
__thread int tls_dllimport __attribute__((visibility("default"))) = 600;
#endif

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* Function to use TLS variables and ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as TREE_USED */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 50;
    tls_common = 999;
    tls_weak = tls_weak / 3;
    tls_hidden = tls_hidden + 1000;
    tls_default = tls_default - 200;
    tls_dllimport = tls_dllimport + 50;
    
    /* Use external TLS variable */
    tls_external = tls_external + 100;
}

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void function_with_local_tls(void) {
    static __thread int tls_local_func = 700;
    tls_local_func = tls_local_func + 10;
}

/* Main entry point */
int main(void) {
    use_tls_variables();
    function_with_local_tls();
    
    /* Additional uses to ensure coverage */
    volatile int sum = tls_used + tls_public + tls_static + tls_common +
                      tls_weak + tls_hidden + tls_default + tls_dllimport;
    
    return 0;
}
