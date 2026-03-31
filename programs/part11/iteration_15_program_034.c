/* Test for emulated TLS attribute copying - C version */

/* Force emulated TLS model */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* Pattern A: Variables with various attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC - non-static at file scope */
__thread int tls_public = 100;

/* Static TLS - not TREE_PUBLIC */
static __thread int tls_static = 200;

/* DECL_COMMON - tentative definition */
__thread int tls_common;  /* No initializer */

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED - hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* Default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 600;

/* Internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 700;

/* DECL_CONTEXT - TLS inside function scope */
void test_function_scope(void) {
    static __thread int tls_func_scope = 800;
    tls_func_scope++;
}

/* DECL_EXTERNAL - external declaration */
extern __thread int tls_external;

/* Complex usage to ensure TREE_USED is set */
void use_all_tls_vars(void) {
    /* Read and write to all TLS variables */
    tls_used += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common += 4;
    tls_weak += 5;
    tls_hidden += 6;
    tls_default += 7;
    tls_protected += 8;
    tls_internal += 9;
    
    /* Take addresses */
    int *ptr1 = &tls_used;
    int *ptr2 = &tls_public;
    int *ptr3 = &tls_static;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    
    test_function_scope();
}

/* DLL import attribute (target-specific) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 900;

int main(void) {
    int sum = 0;
    
    /* Initialize common variable */
    tls_common = 50;
    
    /* Use all TLS variables */
    use_all_tls_vars();
    
    /* Collect results */
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_global_dynamic;
    
    /* Reference external */
    sum += tls_external;
    
    printf("TLS sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
