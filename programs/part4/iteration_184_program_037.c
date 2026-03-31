/* This should trigger emulated TLS code generation */
/* Test case for tree-emutls.cc attribute copying logic */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* TLS variables with various attributes to test DECL_* attribute copying */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static (internal linkage) */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak symbol */
__attribute__((weak)) __thread int tls_weak = 5;

/* Hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 6;

/* Default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_default_vis = 7;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 8;

/* Uninitialized TLS variable */
__thread int tls_uninit;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, simulate with a different attribute */
__thread int tls_dllimport __attribute__((weak));
#endif

/* Definition of previously declared extern */
__thread int tls_extern = 3;

/* Common TLS variable (uninitialized, external linkage) */
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden -= 1;
    tls_used = tls_default + tls_static;
    
    /* Take address to inhibit optimizations */
    int *addr = &tls_default;
    (void)addr; /* Suppress unused warning */
}

/* Another helper to ensure TLS variables are referenced */
int compute_tls_sum(void) {
    int sum = 0;
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default_vis;
    sum += tls_used;
    sum += tls_uninit;
    sum += tls_dllimport;
    sum += tls_common;
    return sum;
}

int main(void) {
    /* Initialize uninitialized TLS variables */
    tls_uninit = 9;
    tls_common = 10;
    
    /* Modify TLS variables in main */
    tls_default += 100;
    tls_static += 200;
    tls_extern += 300;
    
    /* Call helper function */
    modify_tls();
    
    /* Take addresses of TLS variables to ensure they're fully processed */
    int *addr1 = &tls_default;
    int *addr2 = &tls_static;
    int *addr3 = &tls_hidden;
    int *addr4 = &tls_weak;
    
    /* Use addresses to create side effects */
    if (addr1 && addr2 && addr3 && addr4) {
        tls_default_vis = *addr1 + *addr2;
    }
    
    /* Compute and print result */
    int result = compute_tls_sum();
    printf("TLS sum: %d\n", result);
    
    /* Additional operations to ensure all paths are exercised */
    tls_dllimport = result % 100;
    
    /* Check weak symbol */
    if (&tls_weak) {
        tls_weak = result;
    }
    
    printf("Final values:\n");
    printf("  tls_default: %d\n", tls_default);
    printf("  tls_static: %d\n", tls_static);
    printf("  tls_extern: %d\n", tls_extern);
    printf("  tls_weak: %d\n", tls_weak);
    printf("  tls_hidden: %d\n", tls_hidden);
    
    return 0;
}
