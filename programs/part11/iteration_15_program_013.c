/* tls_main.c - Main test file for emulated TLS attribute coverage */

#include <stdio.h>

/* Force emulated TLS model for coverage */
#ifdef __GNUC__
#pragma GCC tls_model emulated
#endif

/* Test DECL_PRESERVE_P with __attribute__((used)) */
__thread int tls_preserve __attribute__((used)) = 42;

/* Test TREE_PUBLIC (non-static, public linkage) */
__thread int tls_public = 100;

/* Test static (non-public) TLS variable */
static __thread int tls_static = 200;

/* Test DECL_COMMON with tentative definition */
__thread int tls_common;  /* No initializer - common symbol */

/* Test DECL_WEAK with weak attribute */
__thread int tls_weak __attribute__((weak)) = 300;

/* Test DECL_VISIBILITY with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* Test DECL_VISIBILITY with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* Test DECL_VISIBILITY with protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 600;

/* Test DECL_VISIBILITY_SPECIFIED explicitly */
__thread int tls_vis_specified __attribute__((visibility("internal"))) = 700;

/* Test TREE_USED by ensuring variable is referenced */
__thread int tls_used;

/* Test DECL_CONTEXT with function-scoped TLS */
static void test_function_context(void) {
    /* Function-local TLS variable */
    static __thread int tls_function_local = 800;
    tls_function_local++;
}

/* Test DECL_EXTERNAL - declare external TLS variable */
extern __thread int tls_external;

/* Test DECL_DLLIMPORT_P - conditionally for Windows targets */
#ifdef _WIN32
extern __thread int __declspec(dllimport) tls_dllimport;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int __attribute__((dllimport)) tls_dllimport;
#endif

/* Complex usage to ensure TREE_USED is set */
__thread int* tls_pointer;

/* Use in asm statement to ensure preservation */
__thread int tls_asm __asm__("tls_asm_var") = 900;

/* Test with different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 1000;

/* Force usage of all variables to set TREE_USED */
void use_all_tls_vars(void) {
    /* Reference all TLS variables */
    tls_preserve++;
    tls_public++;
    tls_static++;
    tls_common = 50;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    tls_protected++;
    tls_vis_specified++;
    tls_used = 150;
    
    /* Take address to ensure complex usage */
    tls_pointer = &tls_public;
    
    /* Use in expression */
    int sum = tls_asm + tls_global_dynamic;
    
    /* Use external variable if available */
#ifdef TLS_EXTERNAL_DEFINED
    tls_external = sum;
#endif
    
    test_function_context();
}

int main(void) {
    use_all_tls_vars();
    
    /* Print something to prevent optimization */
    printf("TLS test: %d %d %d\n", 
           tls_preserve, 
           tls_public, 
           tls_common);
    
    return 0;
}
