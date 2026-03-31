/* Test case specifically designed to trigger TLS emulation attribute copying
   in tree-emutls.cc lines 295-304. This should trigger emulated TLS code generation. */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags:
   -femulated-tls or targeting architecture without native TLS support */

/* 1. TLS variables with different storage classes and linkages */

/* Plain __thread with external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS with internal linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Definition of external TLS variable */
__thread int tls_extern = 3;

/* 2. TLS variables with various GCC attributes */

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 4;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible_default = 6;

/* TLS with used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* Uninitialized TLS variables */
__thread int tls_uninit;
static __thread int tls_static_uninit;
__attribute__((weak)) __thread int tls_weak_uninit;

/* For Windows-like DLL import simulation (cross-compilation scenario) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, just a regular TLS */
__thread int tls_dllimport = 8;
#endif

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden = tls_visible_default + tls_used;
    
    /* Use weak TLS variable */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Use uninitialized TLS */
    tls_uninit = 42;
    tls_static_uninit = tls_default;
    
    /* Take address of TLS variable to inhibit optimizations */
    int *tls_ptr = &tls_visible_default;
    *tls_ptr += 5;
}

/* Dummy function that takes TLS address */
void use_tls_ptr(int *ptr) {
    if (ptr) {
        *ptr += 1;
    }
}

int main(void) {
    int sum = 0;
    
    /* 3. Use TLS variables in main() */
    
    /* Basic operations */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible_default;
    sum += tls_used;
    sum += tls_dllimport;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS values */
    tls_default = 100;
    tls_static = 200;
    tls_extern = 300;
    
    /* Call helper function */
    modify_tls();
    
    /* Take address of TLS variable */
    int *addr_default = &tls_default;
    int *addr_hidden = &tls_hidden;
    
    use_tls_ptr(addr_default);
    use_tls_ptr(addr_hidden);
    
    /* Compute final result using TLS variables */
    int result = tls_default + tls_static + tls_extern + tls_hidden + 
                 tls_visible_default + tls_used + tls_uninit;
    
    printf("Result: %d\n", result);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_static_uninit: %d\n", tls_static_uninit);
    
    return 0;
}
