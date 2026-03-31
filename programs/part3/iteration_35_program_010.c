/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by targeting ARM without TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

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

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default_vis __attribute__((visibility("default"))) = 500;

/* DECL_DLLIMPORT_P: Windows dllimport (use relevant attribute) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void use_function_tls(void) {
    static __thread int tls_in_function = 600;
    tls_in_function++;
}

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read and write to ensure usage */
    tls_used += 1;
    tls_public = tls_used * 2;
    tls_static = tls_public + 1;
    tls_common = 999;
    tls_weak = tls_common;
    tls_hidden = tls_weak + 1;
    tls_default_vis = tls_hidden * 2;
    tls_dllimport = 1234;
    
    /* Use function-local TLS */
    use_function_tls();
    
    /* Use external TLS */
    tls_external = 777;
}

/* Main function that uses all TLS variables */
int main(void) {
    reference_all_tls();
    
    /* Additional usage patterns */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default_vis;
    sum += tls_dllimport;
    
    /* Call function that uses TLS */
    use_function_tls();
    
    return 0;
}
