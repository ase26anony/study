/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by targeting ARM without TLS support */
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

/* DECL_DLLIMPORT_P: Windows dllimport (use appropriate attribute) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* Function to use TLS variables - ensures TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    tls_used += 1;
    tls_public = tls_static + 1;
    tls_common = tls_weak;
    tls_hidden = tls_default;
    tls_dllimport = 600;
    
    /* Use external TLS variable */
    tls_external = tls_public + tls_used;
}

/* DECL_CONTEXT: TLS variable inside a function (block scope) */
void function_with_tls(void) {
    static __thread int tls_in_function = 700;
    tls_in_function++;
}

/* Helper function that returns addresses to prevent optimization */
int* get_tls_addresses(void) {
    static int* addresses[] = {
        &tls_used,
        &tls_public,
        &tls_static,
        &tls_common,
        &tls_weak,
        &tls_hidden,
        &tls_default,
        &tls_dllimport,
        &tls_external
    };
    return addresses[0];
}
