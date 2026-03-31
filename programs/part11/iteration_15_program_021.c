/* Test for emulated TLS attribute copying - Main file */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* DECL_PRESERVE_P: Used attribute ensures preservation */
__thread int tls_used_attr __attribute__((used)) = 42;
__thread int tls_not_used_attr = 100;

/* TREE_USED: Variables that will be referenced */
__thread int tls_used_var = 1;
static __thread int tls_static_used = 2;

/* TREE_PUBLIC: Public vs static */
__thread int tls_public = 3;
static __thread int tls_static = 4;

/* DECL_EXTERNAL: Extern declaration (defined in another file) */
extern __thread int tls_extern;

/* DECL_COMMON: Tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: Weak symbol */
__thread int tls_weak __attribute__((weak)) = 5;

/* DECL_VISIBILITY: Various visibility attributes */
__thread int tls_default __attribute__((visibility("default"))) = 6;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 7;
__thread int tls_protected __attribute__((visibility("protected"))) = 8;
#ifdef __GNUC__
__thread int tls_internal __attribute__((visibility("internal"))) = 9;
#endif

/* DECL_VISIBILITY_SPECIFIED: Explicit vs implicit visibility */
__thread int tls_vis_specified __attribute__((visibility("hidden"))) = 10;
__thread int tls_vis_unspecified = 11;

/* DECL_DLLIMPORT_P: DLL import on supported targets */
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_dllimport __declspec(dllimport);
#else
/* Simulate with attribute on non-Windows */
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function-scoped TLS (tests DECL_CONTEXT) */
void test_function_context(void) {
    static __thread int tls_func_static = 12;
    __thread int tls_func_auto = 13;
    
    tls_func_static++;
    tls_func_auto++;
    
    /* Reference to mark as used */
    printf("Function TLS: %d, %d\n", tls_func_static, tls_func_auto);
}

/* Complex usage patterns to ensure processing */
void complex_tls_usage(void) {
    /* Address taking */
    int *ptr1 = &tls_public;
    int *ptr2 = &tls_static;
    
    /* Inline asm reference (forces TREE_USED) */
    asm volatile("" : : "r"(tls_used_var) : "memory");
    
    /* Non-trivial expressions */
    tls_public = tls_static * 2 + tls_used_var;
    tls_common = tls_weak ^ tls_default;
    
    /* Volatile access */
    volatile __thread int tls_volatile = 99;
    tls_volatile++;
}

/* Different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 14;
__thread int tls_local_dynamic __attribute__((tls_model("local-dynamic"))) = 15;
__thread int tls_initial_exec __attribute__((tls_model("initial-exec"))) = 16;
__thread int tls_local_exec __attribute__((tls_model("local-exec"))) = 17;

int main(void) {
    /* Ensure all TLS variables are TREE_USED */
    tls_used_var = 100;
    tls_static_used = 200;
    tls_public = 300;
    tls_static = 400;
    tls_extern = 500;  /* Defined in another file */
    tls_common = 600;
    tls_weak = 700;
    tls_default = 800;
    tls_hidden = 900;
    tls_protected = 1000;
#ifdef __GNUC__
    tls_internal = 1100;
#endif
    tls_vis_specified = 1200;
    tls_vis_unspecified = 1300;
    
    /* Test function context */
    test_function_context();
    
    /* Complex usage */
    complex_tls_usage();
    
    /* Use all TLS models */
    tls_global_dynamic++;
    tls_local_dynamic++;
    tls_initial_exec++;
    tls_local_exec++;
    
    /* Take addresses to ensure they're processed */
    void *addrs[] = {
        &tls_used_attr,
        &tls_not_used_attr,
        &tls_used_var,
        &tls_static_used,
        &tls_public,
        &tls_static,
        &tls_extern,
        &tls_common,
        &tls_weak,
        &tls_default,
        &tls_hidden,
        &tls_protected,
#ifdef __GNUC__
        &tls_internal,
#endif
        &tls_vis_specified,
        &tls_vis_unspecified,
        &tls_global_dynamic,
        &tls_local_dynamic,
        &tls_initial_exec,
        &tls_local_exec
    };
    
    /* Calculate sum for observable output */
    int sum = 0;
    sum += tls_used_var;
    sum += tls_static_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_default;
    sum += tls_hidden;
    sum += tls_protected;
#ifdef __GNUC__
    sum += tls_internal;
#endif
    sum += tls_vis_specified;
    sum += tls_vis_unspecified;
    sum += tls_global_dynamic;
    sum += tls_local_dynamic;
    sum += tls_initial_exec;
    sum += tls_local_exec;
    
    printf("TLS sum: %d\n", sum);
    printf("Address array size: %zu\n", sizeof(addrs)/sizeof(addrs[0]));
    
    return sum > 0 ? 0 : 1;
}
