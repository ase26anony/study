/* Test for EMUTLS attribute copying - Main file */
#include <stdio.h>

/* Force EMUTLS by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

/* 1. DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* 2. TREE_PUBLIC: non-static (public) TLS */
__thread int tls_public = 100;

/* 3. TREE_PUBLIC: static (non-public) TLS */
static __thread int tls_static = 200;

/* 4. DECL_COMMON: TLS without initializer (common linkage) */
__thread int tls_common;

/* 5. DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* 6. DECL_VISIBILITY_SPECIFIED: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* 7. DECL_VISIBILITY_SPECIFIED: default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* 8. DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* 9. DECL_DLLIMPORT_P: Windows dllimport (simulated with attribute) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* For non-Windows, use a visibility attribute to simulate */
__thread int tls_dllimport __attribute__((visibility("default"))) = 600;
#endif

/* Function with local TLS variable (different DECL_CONTEXT) */
void function_with_tls(void) {
    /* 10. DECL_CONTEXT: TLS inside function scope */
    static __thread int tls_in_function = 700;
    
    tls_in_function++;
    printf("TLS in function: %d\n", tls_in_function);
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read/write to mark as used */
    tls_used += 1;
    tls_public = tls_used * 2;
    tls_static = tls_public + 1;
    tls_common = 999;
    tls_weak = tls_common - 100;
    tls_hidden *= 2;
    tls_default /= 2;
    tls_external = 1234;  /* Use external TLS */
    tls_dllimport = 5678;
    
    printf("tls_used: %d\n", tls_used);
    printf("tls_public: %d\n", tls_public);
    printf("tls_static: %d\n", tls_static);
    printf("tls_common: %d\n", tls_common);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_default: %d\n", tls_default);
    printf("tls_external: %d\n", tls_external);
    printf("tls_dllimport: %d\n", tls_dllimport);
}

int main(void) {
    printf("Testing EMUTLS attribute copying...\n");
    
    /* Reference all TLS variables */
    reference_all_tls();
    
    /* Call function with local TLS */
    function_with_tls();
    function_with_tls();  /* Call twice to ensure persistence */
    
    /* Use external TLS variable */
    tls_external += 100;
    printf("Modified tls_external: %d\n", tls_external);
    
    return 0;
}
