/* tls_main.c - Main test file for emulated TLS attribute coverage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS for all variables unless overridden */
#pragma GCC tls_model("emulated")

/* Pattern A: Variables with DECL_PRESERVE_P set via __attribute__((used)) */
__thread int tls_used_attr __attribute__((used)) = 42;
__thread int tls_unused_attr = 100;

/* Pattern B: Variables in different scopes for DECL_CONTEXT */
/* File scope - default context */
__thread int tls_file_scope = 1;

/* Static function with TLS variable */
static void helper_func(void) {
    static __thread int tls_static_func_scope = 2;
    tls_static_func_scope++;
}

/* Pattern C: TREE_USED - variables that are referenced */
__thread int tls_used_referenced = 3;
__thread int tls_unused_declared = 4;

/* Pattern D: TREE_PUBLIC and DECL_EXTERNAL */
/* Public TLS variable (non-static) */
__thread int tls_public = 5;
/* Static TLS variable (not public) */
static __thread int tls_static = 6;

/* Pattern E: DECL_COMMON - tentative definitions */
__thread int tls_common;  /* Tentative definition - should be common */
__thread int tls_initialized = 7;  /* Not common - has initializer */

/* Pattern F: DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 8;
__thread int tls_strong = 9;

/* Pattern G: DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 10;
__thread int tls_default_vis __attribute__((visibility("default"))) = 11;
__thread int tls_protected __attribute__((visibility("protected"))) = 12;
__thread int tls_internal __attribute__((visibility("internal"))) = 13;
/* Variable without explicit visibility attribute */
__thread int tls_no_vis_attr = 14;

/* Pattern H: DECL_DLLIMPORT_P - target specific */
#ifdef _WIN32
__declspec(dllimport) extern __thread int tls_imported;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_imported __attribute__((dllimport));
#else
/* On non-Windows, we'll simulate with a regular extern */
extern __thread int tls_imported;
#endif

/* Pattern I: C++11 thread_local (will be in separate C++ file) */

/* Pattern J: Address taking and complex usage for TREE_USED */
__thread int tls_for_address;
__thread int tls_for_asm;

/* Pattern K: Weak alias */
extern __thread int tls_weak_alias __attribute__((weak, alias("tls_strong")));

/* Function that uses TLS variables to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference used variables */
    tls_used_referenced += 1;
    tls_used_attr += 1;
    
    /* Use file scope variable */
    tls_file_scope = tls_public + tls_static;
    
    /* Use common variable */
    tls_common = 100;
    
    /* Use weak variable */
    if (&tls_weak) {
        tls_weak = 50;
    }
    
    /* Use visibility-controlled variables */
    tls_hidden = 20;
    tls_default_vis = 21;
    tls_protected = 22;
    tls_internal = 23;
    tls_no_vis_attr = 24;
    
    /* Take address */
    int *ptr = &tls_for_address;
    *ptr = 30;
    
    /* Use in inline asm to ensure it's marked used */
    __asm__ volatile ("" : : "r"(&tls_for_asm));
    
    /* Call helper to use function-scope TLS */
    helper_func();
    
    /* Use imported variable if available */
#ifdef TLS_IMPORTED_DEFINED
    tls_imported = 40;
#endif
}

/* Main function that exercises all TLS variables */
int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    tls_used_attr = 1;
    tls_unused_attr = 2;
    tls_file_scope = 3;
    tls_used_referenced = 4;
    tls_unused_declared = 5;
    tls_public = 6;
    tls_static = 7;
    tls_common = 8;
    tls_initialized = 9;
    tls_weak = 10;
    tls_strong = 11;
    tls_hidden = 12;
    tls_default_vis = 13;
    tls_protected = 14;
    tls_internal = 15;
    tls_no_vis_attr = 16;
    tls_for_address = 17;
    tls_for_asm = 18;
    
    /* Call function that uses variables */
    use_tls_variables();
    
    /* Sum all values to ensure they're used */
    sum += tls_used_attr;
    sum += tls_unused_attr;
    sum += tls_file_scope;
    sum += tls_used_referenced;
    sum += tls_unused_declared;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_initialized;
    sum += tls_weak;
    sum += tls_strong;
    sum += tls_hidden;
    sum += tls_default_vis;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_no_vis_attr;
    sum += tls_for_address;
    sum += tls_for_asm;
    
    printf("TLS sum: %d\n", sum);
    
    /* Return non-zero if sum doesn't match expected */
    return (sum != 0) ? 0 : 1;
}
