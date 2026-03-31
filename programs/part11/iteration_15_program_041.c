/* Test for emulated TLS attribute copying - Main file */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* DECL_PRESERVE_P: Used attribute ensures preservation */
__thread int tls_used_attr __attribute__((used)) = 42;
__thread int tls_not_used = 0;

/* TREE_USED: Variables that will be referenced */
__thread int tls_used1 = 100;
__thread int tls_used2 = 200;
static __thread int tls_static_used = 300;

/* TREE_PUBLIC: Public (non-static) TLS */
__thread int tls_public = 1;
static __thread int tls_non_public = 2;

/* DECL_EXTERNAL: External declaration */
extern __thread int tls_external;

/* DECL_COMMON: Tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: Weak TLS variables */
__thread int tls_weak __attribute__((weak)) = 999;

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_default_vis __attribute__((visibility("default"))) = 10;
__thread int tls_hidden_vis __attribute__((visibility("hidden"))) = 20;
__thread int tls_protected_vis __attribute__((visibility("protected"))) = 30;

/* DECL_VISIBILITY_SPECIFIED: Explicitly specified visibility */
__thread int tls_vis_specified __attribute__((visibility("internal"))) = 40;

/* DECL_DLLIMPORT_P: DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#else
/* On non-Windows, use GNU attribute */
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_CONTEXT: Different scopes */
static void func_with_tls(void) {
    /* Function-scoped TLS */
    static __thread int tls_func_scope = 50;
    tls_func_scope++;
}

/* Complex usage patterns to ensure processing */
__thread int* tls_pointer;
__thread int tls_array[10];

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Force usage of all TLS variables */
    volatile int sum = 0;
    
    sum += tls_used_attr;
    sum += tls_not_used;
    sum += tls_used1;
    sum += tls_used2;
    sum += tls_static_used;
    sum += tls_public;
    sum += tls_non_public;
    sum += tls_external;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_default_vis;
    sum += tls_hidden_vis;
    sum += tls_protected_vis;
    sum += tls_vis_specified;
    sum += tls_dllimport;
    
    /* Address-taking */
    tls_pointer = &tls_used1;
    tls_array[0] = sum;
    
    /* Call function with TLS */
    func_with_tls();
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(sum) : "memory");
}

/* Different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 123;

int main(void) {
    /* Initialize some TLS variables */
    tls_used1 = 1000;
    tls_used2 = 2000;
    tls_common = 500;
    
    /* Reference all TLS variables */
    reference_all_tls();
    
    /* Use TLS in complex expressions */
    int result = tls_used1 + tls_used2 + tls_external;
    result += tls_common * 2;
    result -= tls_weak;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Use function-scoped TLS */
    func_with_tls();
    
    return 0;
}
