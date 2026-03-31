/* Test for emulated TLS attribute copying - Main file */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* DECL_PRESERVE_P: Used attribute ensures preservation */
__thread int tls_used_attr __attribute__((used)) = 42;
__thread int tls_not_used_attr = 100;

/* TREE_PUBLIC: Public (non-static) TLS variables */
__thread int tls_public = 1;
static __thread int tls_static = 2;

/* DECL_COMMON: Tentative definition (common symbol) */
__thread int tls_common;  /* No initializer */

/* DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 3;

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_default __attribute__((visibility("default"))) = 4;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 5;
__thread int tls_protected __attribute__((visibility("protected"))) = 6;

/* DECL_CONTEXT: Different scopes */
static void func_with_tls(void) {
    /* Function-scope TLS */
    static __thread int tls_func_scope = 7;
    tls_func_scope++;
}

/* DECL_EXTERNAL: External declaration (defined in another file) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: DLL import attribute (target-specific) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Complex usage to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    volatile int sum = 0;
    
    sum += tls_used_attr;
    sum += tls_not_used_attr;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_default;
    sum += tls_hidden;
    sum += tls_protected;
    
    /* Take addresses to force more complex handling */
    int* ptr1 = &tls_public;
    int* ptr2 = &tls_static;
    
    /* Use in inline asm to prevent optimization */
    asm volatile("" : "+r"(sum) : "r"(ptr1), "r"(ptr2));
    
    func_with_tls();
}

/* Test different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 8;
__thread int tls_local_exec __attribute__((tls_model("local-exec"))) = 9;

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 10;
    
    /* Use external TLS variable */
    tls_external = 20;
    
    /* Use all TLS variables */
    use_tls_variables();
    
    /* Print results to prevent optimization */
    printf("TLS values: %d %d %d %d %d %d %d %d %d %d %d\n",
           tls_used_attr, tls_not_used_attr, tls_public,
           tls_static, tls_common, tls_weak, tls_default,
           tls_hidden, tls_protected, tls_external,
           tls_global_dynamic + tls_local_exec);
    
    return 0;
}
