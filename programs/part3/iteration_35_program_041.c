/* Test for EMUTLS attribute copying - C file */
#include <stdio.h>

/* Force EMUTLS by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_preserve __attribute__((used)) = 42;

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
__thread int tls_default_vis __attribute__((visibility("default"))) = 500;

/* DECL_DLLIMPORT_P: dllimport attribute (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* Function-scoped TLS variable (different DECL_CONTEXT) */
void use_function_tls(void) {
    /* TLS variable inside function scope */
    static __thread int tls_function_scope = 600;
    tls_function_scope++;
    printf("Function TLS: %d\n", tls_function_scope);
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read and write to each TLS variable */
    tls_preserve += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common = tls_public + tls_static;
    tls_weak += 4;
    tls_hidden += 5;
    tls_default_vis += 6;
    tls_dllimport += 7;
    tls_external += 8;
    
    /* Use the values to prevent optimization */
    printf("Preserve: %d, Public: %d, Static: %d\n", 
           tls_preserve, tls_public, tls_static);
    printf("Common: %d, Weak: %d, Hidden: %d\n",
           tls_common, tls_weak, tls_hidden);
    printf("DefaultVis: %d\n", tls_default_vis);
}
