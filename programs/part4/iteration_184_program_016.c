/* This should trigger emulated TLS code generation */
/* Test case for tree-emutls.cc attribute copying logic */

#include <stdio.h>

/* TLS variables with various attributes to test DECL_* property copying */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* External definition */
__thread int tls_extern = 3;

/* Weak symbol */
__attribute__((weak)) __thread int tls_weak = 4;

/* Weak undefined */
__attribute__((weak)) extern __thread int tls_weak_undef;

/* Hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* Default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_default_vis = 6;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use a different attribute that might trigger similar paths */
__attribute__((weak)) __thread int tls_dllimport = 8;
#endif

/* Uninitialized TLS */
__thread int tls_uninit;

/* Common symbol simulation */
extern __thread int tls_common;
__thread int tls_common = 9;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 1;
    tls_static *= 2;
    tls_hidden -= 1;
    tls_used = tls_default_vis;
    
    /* Use weak TLS if available */
    if (&tls_weak_undef) {
        tls_weak_undef = 100;
    }
    
    /* Take address to inhibit optimizations */
    volatile int *addr = &tls_uninit;
    *addr = 42;
}

/* Another function that takes TLS address */
int* get_tls_address(void) {
    /* Taking address forces symbol reference */
    return &tls_default;
}

int main(void) {
    int sum = 0;
    
    /* Initialize uninitialized TLS */
    tls_uninit = 10;
    
    /* Use all TLS variables to prevent elimination */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default_vis;
    sum += tls_used;
    sum += tls_dllimport;
    sum += tls_uninit;
    sum += tls_common;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_default_vis + tls_used + tls_dllimport +
          tls_uninit + tls_common;
    
    printf("Modified sum: %d\n", sum);
    
    /* Force symbol references through address taking */
    volatile int *addr1 = get_tls_address();
    volatile int *addr2 = &tls_static;
    volatile int *addr3 = &tls_hidden;
    
    /* Use addresses to create side effects */
    if (addr1 && addr2 && addr3) {
        *addr1 += 1;
        printf("Final tls_default: %d\n", tls_default);
    }
    
    return 0;
}
