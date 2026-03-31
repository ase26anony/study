/* Main test file for emulated TLS attribute coverage */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS model for coverage */
#pragma GCC tls_model emulated

/* DECL_PRESERVE_P: Use __attribute__((used)) */
__thread int tls_used_attr __attribute__((used)) = 42;

/* TREE_PUBLIC: Non-static (public) TLS variable */
__thread int tls_public = 100;

/* DECL_COMMON: Tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 200;

/* DECL_VISIBILITY: Hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* DECL_VISIBILITY_SPECIFIED: Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* Internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 500;

/* Static (non-public) TLS variable - tests !TREE_PUBLIC */
static __thread int tls_static = 600;

/* DECL_CONTEXT: TLS variable inside function scope */
void test_function_scope(void) {
    static __thread int tls_func_static = 700;
    tls_func_static++;
}

/* TREE_USED: Ensure variables are referenced */
void use_all_tls_vars(void) {
    /* Reference all TLS variables to mark them as used */
    tls_used_attr += 1;
    tls_public += 2;
    tls_common = tls_public + 10;
    tls_weak += 3;
    tls_hidden += 4;
    tls_protected += 5;
    tls_internal += 6;
    tls_static += 7;
    
    /* Take addresses to force more complex usage */
    int *ptr1 = &tls_used_attr;
    int *ptr2 = &tls_public;
    int *ptr3 = &tls_weak;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
}

/* Extern declaration from another file */
extern __thread int tls_extern;
extern __thread int tls_extern_defined;

/* DLL import simulation (conditional) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 800;

int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    tls_common = 50;  /* Initialize tentative definition */
    
    use_all_tls_vars();
    test_function_scope();
    
    /* Collect values for verification */
    sum += tls_used_attr;
    sum += tls_public;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_extern_defined;
    
#ifdef _WIN32
    sum += tls_dllimport;
#endif
    
    sum += tls_global_dynamic;
    
    /* Print result to prevent optimization */
    printf("TLS sum: %d\n", sum);
    
    return 0;
}
