/* Test for emulated TLS attribute copying - Main file */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* DECL_PRESERVE_P: Used attribute ensures preservation */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: Non-static (public) TLS variable */
__thread int tls_public = 100;

/* DECL_COMMON: Tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 200;

/* DECL_VISIBILITY_SPECIFIED: Hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* DECL_VISIBILITY: Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* DECL_VISIBILITY: Internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 500;

/* Static TLS variable (not TREE_PUBLIC) */
static __thread int tls_static = 600;

/* DECL_CONTEXT: TLS variable inside function scope */
void test_function_scope(void) {
    static __thread int tls_func_scope = 700;
    tls_func_scope++;
}

/* Extern declaration (will be defined in another file) */
extern __thread int tls_extern;

/* For DECL_DLLIMPORT_P - conditionally use dllimport */
#ifdef _WIN32
extern __thread int tls_import __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_import __attribute__((dllimport));
#endif

/* Complex usage to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    volatile int sum = 0;
    
    sum += tls_used;
    sum += tls_public;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_static;
    sum += tls_extern;
    
    #if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
    sum += tls_import;
    #endif
    
    /* Take addresses to force more complex handling */
    void* addrs[] = {
        (void*)&tls_used,
        (void*)&tls_public,
        (void*)&tls_common,
        (void*)&tls_weak,
        (void*)&tls_hidden,
        (void*)&tls_protected,
        (void*)&tls_internal,
        (void*)&tls_static,
        (void*)&tls_extern,
        #if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
        (void*)&tls_import,
        #endif
        NULL
    };
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(sum) : "memory");
}

/* Test different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 800;
__thread int tls_local_dynamic __attribute__((tls_model("local-dynamic"))) = 900;
__thread int tls_initial_exec __attribute__((tls_model("initial-exec"))) = 1000;
__thread int tls_local_exec __attribute__((tls_model("local-exec"))) = 1100;

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 1200;
    
    /* Test function scope TLS */
    test_function_scope();
    
    /* Use all TLS variables */
    use_tls_variables();
    
    /* Compute a result to verify functionality */
    int result = 
        tls_used + tls_public + tls_common + tls_weak +
        tls_hidden + tls_protected + tls_internal + tls_static +
        tls_extern + tls_global_dynamic + tls_local_dynamic +
        tls_initial_exec + tls_local_exec;
    
    #if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
    result += tls_import;
    #endif
    
    printf("TLS test result: %d\n", result);
    
    /* Return non-zero if any TLS variable has wrong value */
    if (result != (42 + 100 + 1200 + 200 + 300 + 400 + 500 + 600 + 
                  1300 + 800 + 900 + 1000 + 1100
                  #if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
                  + 1400
                  #endif
                  )) {
        return 1;
    }
    
    return 0;
}
