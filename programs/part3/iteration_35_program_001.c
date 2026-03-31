/* Test for EMUTLS attribute copying - C file */
#include <stdio.h>

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

/* DECL_VISIBILITY_SPECIFIED: default visibility (explicit) */
__thread int tls_default_vis __attribute__((visibility("default"))) = 500;

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: Windows dllimport (use appropriate attribute) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* For non-Windows, we'll use a different attribute to test the code path */
__thread int tls_dllimport __attribute__((weak)) = 600;
#endif

/* Function using TLS variables to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as TREE_USED */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 50;
    tls_common = 999;
    tls_weak = tls_weak / 3;
    tls_hidden = tls_hidden + 1000;
    tls_default_vis = tls_default_vis * 3;
    tls_external = tls_external + 1;
    tls_dllimport = tls_dllimport - 100;
    
    /* Print to prevent optimization */
    printf("C TLS values: %d %d %d %d %d %d %d %d %d\n", 
           tls_used, tls_public, tls_static, tls_common, 
           tls_weak, tls_hidden, tls_default_vis, 
           tls_external, tls_dllimport);
}

/* DECL_CONTEXT: TLS inside a function (local scope) */
void function_with_local_tls(void) {
    static __thread int local_function_tls = 700;
    local_function_tls++;
    printf("Local function TLS: %d\n", local_function_tls);
}

int main(void) {
    use_tls_variables();
    function_with_local_tls();
    return 0;
}
