/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS emulation attribute copying logic */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

#include <stdio.h>

/* Force declaration attributes to be set on TLS variables */

/* Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS with internal linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* External definition */
__thread int tls_extern = 3;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 4;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible_default = 6;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but we can still set the attribute */
__attribute__((dllimport)) __thread int tls_dllimport = 8;
#endif

/* Uninitialized TLS variables with different attributes */
__thread int tls_uninit;
__attribute__((weak)) __thread int tls_weak_uninit;
static __thread int tls_static_uninit;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden -= 3;
    tls_weak = tls_default + tls_static;
    
    /* Take address to inhibit optimizations */
    int *ptr1 = &tls_default;
    int *ptr2 = &tls_hidden;
    
    /* Use pointers to create side effects */
    if (ptr1 != ptr2) {
        tls_used = *ptr1 + *ptr2;
    }
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *addr_array[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_visible_default,
        &tls_used,
        &tls_uninit,
        &tls_weak_uninit,
        &tls_static_uninit
    };
    
    /* Use addresses to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += *addr_array[i];
    }
}

int main(void) {
    /* Initialize uninitialized TLS variables */
    tls_uninit = 100;
    tls_weak_uninit = 200;
    tls_static_uninit = 300;
    
    /* Use TLS variables in main */
    int result = tls_default + tls_static + tls_extern;
    result += tls_weak + tls_hidden + tls_visible_default;
    result += tls_used + tls_uninit + tls_weak_uninit + tls_static_uninit;
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS through helper function */
    modify_tls();
    
    /* Recalculate after modification */
    result = tls_default + tls_static + tls_extern;
    result += tls_weak + tls_hidden + tls_visible_default;
    result += tls_used + tls_uninit + tls_weak_uninit + tls_static_uninit;
    
    printf("After modify_tls: %d\n", result);
    
    /* Take addresses to ensure symbols are required */
    take_tls_addresses();
    
    /* Final computation using all TLS variables */
    int final = 
        tls_default * 1 +
        tls_static * 2 +
        tls_extern * 3 +
        tls_weak * 4 +
        tls_hidden * 5 +
        tls_visible_default * 6 +
        tls_used * 7 +
        tls_uninit * 8 +
        tls_weak_uninit * 9 +
        tls_static_uninit * 10;
    
    printf("Weighted sum: %d\n", final);
    
    /* Return value based on TLS state */
    return (final > 10000) ? 0 : 1;
}
