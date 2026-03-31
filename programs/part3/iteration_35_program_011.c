/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS by using non-TLS-supporting target flags */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_preserved __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable with initializer */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED: explicit hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED: explicit default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_DLLIMPORT_P: Windows dllimport (use appropriate attribute for target) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with appropriate attribute if available */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_CONTEXT: TLS variable inside a function (local context) */
void use_local_tls(void) {
    static __thread int tls_local_func = 600;
    tls_local_func++;
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_vars(void) {
    /* Read/write operations to mark as used */
    tls_preserved += 1;
    tls_public = tls_preserved * 2;
    tls_static -= 5;
    tls_common = 999;
    tls_weak = tls_public + tls_static;
    tls_hidden *= 2;
    tls_default /= 2;
    tls_dllimport = 1234;
    
    use_local_tls();
}

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Function that uses external TLS */
int use_external_tls(void) {
    return tls_external * 2;
}
