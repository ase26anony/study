/* Test TLS variables with various attributes for EMUTLS transformation */

/* Force EMUTLS by using a target without native TLS support */
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

/* DECL_VISIBILITY_SPECIFIED: visibility specified */
__thread int tls_vis_specified __attribute__((visibility("internal"))) = 600;

/* DECL_DLLIMPORT_P: Windows dllimport (use appropriate attribute for target) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with appropriate attribute if available */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function to use TLS variables - ensures TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as TREE_USED */
    tls_used += 1;
    tls_public = tls_static + 1;
    tls_common = tls_weak;
    tls_hidden = tls_default;
    tls_vis_specified = tls_dllimport + 1;
    
    /* Use static TLS variable within function scope */
    static __thread int tls_function_scope = 700;
    tls_function_scope++;
}

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Main function that uses all TLS variables */
int main(void) {
    use_tls_variables();
    
    /* Additional uses to ensure TREE_USED */
    tls_public = tls_used * 2;
    tls_external = tls_public + tls_common;
    
    return tls_public > 0 ? 0 : 1;
}
