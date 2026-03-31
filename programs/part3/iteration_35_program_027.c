/* Test for EMUTLS attribute copying - Main file */
#include <stdio.h>

/* Force EMUTLS transformation by targeting non-TLS architecture */
#if defined(__GNUC__) && !defined(__EMUTLS_FORCE__)
#define EMUTLS_FORCE __attribute__((target("arch=armv5te")))
#else
#define EMUTLS_FORCE
#endif

/* 1. DECL_PRESERVE_P: used attribute */
EMUTLS_FORCE __thread int tls_preserved EMUTLS_FORCE __attribute__((used));

/* 2. TREE_PUBLIC: non-static (public) TLS variable with initializer */
EMUTLS_FORCE __thread int tls_public = 42;

/* 3. TREE_PUBLIC: static (non-public) TLS variable */
EMUTLS_FORCE static __thread int tls_static = 100;

/* 4. DECL_COMMON: TLS variable without initializer (common linkage) */
EMUTLS_FORCE __thread int tls_common;

/* 5. DECL_WEAK: weak TLS variable */
EMUTLS_FORCE __thread int tls_weak __attribute__((weak)) = 200;

/* 6. DECL_VISIBILITY: hidden visibility */
EMUTLS_FORCE __thread int tls_hidden 
    __attribute__((visibility("hidden"))) = 300;

/* 7. DECL_VISIBILITY: default visibility (explicit) */
EMUTLS_FORCE __thread int tls_default 
    __attribute__((visibility("default"))) = 400;

/* 8. DECL_EXTERNAL: external declaration (defined in another file) */
extern EMUTLS_FORCE __thread int tls_external;

/* 9. DECL_DLLIMPORT_P: Windows dllimport (simulated with attribute) */
#ifdef _WIN32
extern EMUTLS_FORCE __thread int tls_dllimport __declspec(dllimport);
#else
/* Simulate with visibility on non-Windows */
extern EMUTLS_FORCE __thread int tls_dllimport 
    __attribute__((visibility("default")));
#endif

/* Function using TLS variables to ensure TREE_USED is set */
EMUTLS_FORCE void use_tls_variables(void) {
    /* Reference all TLS variables to mark them TREE_USED */
    tls_preserved = 1;
    tls_public = tls_public + 1;
    tls_static = tls_static * 2;
    tls_common = 50;
    tls_weak = tls_weak - 1;
    tls_hidden = tls_hidden / 2;
    tls_default = tls_default + 100;
    tls_external = tls_external + 5;
    
    /* Use the variables to prevent optimization */
    printf("Preserved: %d\n", tls_preserved);
    printf("Public: %d\n", tls_public);
    printf("Static: %d\n", tls_static);
    printf("Common: %d\n", tls_common);
    printf("Weak: %d\n", tls_weak);
    printf("Hidden: %d\n", tls_hidden);
    printf("Default: %d\n", tls_default);
    printf("External: %d\n", tls_external);
}

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
EMUTLS_FORCE void function_with_local_tls(void) {
    static EMUTLS_FORCE __thread int tls_local_func = 500;
    tls_local_func++;
    printf("Local TLS in function: %d\n", tls_local_func);
}

int main(void) {
    /* Ensure all TLS variables are used */
    use_tls_variables();
    function_with_local_tls();
    
    /* Use external variables */
    tls_dllimport = 999;
    printf("DLLImport: %d\n", tls_dllimport);
    
    return 0;
}
