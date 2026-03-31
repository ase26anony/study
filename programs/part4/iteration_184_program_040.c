/* Test case to trigger TLS emulation attribute copying in tree-emutls.cc
   Specifically targets lines 295-304 copying DECL_* attributes
   Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

#include <stdio.h>

/* Force emulated TLS code path */
/* This should trigger emulated TLS code generation */

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

/* TLS with default visibility */
__attribute__((visibility("default"))) __thread int tls_default_vis = 6;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport but we'll declare it anyway */
__thread int tls_dllimport = 8;
#endif

/* Common TLS (uninitialized, external linkage) */
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden -= 1;
    tls_used = tls_default + tls_static;
    
    /* Take address to inhibit optimizations */
    int *ptr = &tls_default;
    *ptr += 1;
}

/* Another helper to use weak TLS */
int use_weak_tls(void) {
    if (&tls_weak != NULL) {
        return tls_weak++;
    }
    return 0;
}

int main(void) {
    int sum = 0;
    
    /* Initialize common TLS */
    tls_common = 9;
    
    /* Use all TLS variables to prevent elimination */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default_vis;
    sum += tls_used;
    sum += tls_dllimport;
    sum += tls_common;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify through helper function */
    modify_tls();
    
    /* Use weak TLS */
    sum += use_weak_tls();
    
    /* More operations with addresses */
    int *addr1 = &tls_static;
    int *addr2 = &tls_hidden;
    int *addr3 = &tls_common;
    
    *addr1 += 100;
    *addr2 += 200;
    *addr3 += 300;
    
    /* Final computation */
    sum = tls_default + tls_static + tls_extern + tls_hidden + 
          tls_default_vis + tls_used + tls_dllimport + tls_common;
    
    printf("Final sum: %d\n", sum);
    
    /* Conditional use of address to create side effect */
    if (addr1 != NULL) {
        printf("Address taken: %p\n", (void*)addr1);
    }
    
    return 0;
}
