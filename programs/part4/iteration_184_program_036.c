/* This should trigger emulated TLS code generation */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

#include <stdio.h>

/* TLS variables with various attributes to trigger attribute copying in tree-emutls.cc */

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
/* On non-Windows, we can't truly test dllimport, but declare it anyway */
__thread int tls_dllimport = 8;
#endif

/* Uninitialized TLS variable */
__thread int tls_uninitialized;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden -= 3;
    tls_used = tls_default + tls_static;
    
    /* Take address to inhibit optimizations */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    
    /* Use addresses to create side effects */
    if (addr1 != addr2) {
        tls_weak = *addr1 + *addr2;
    }
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Taking addresses forces symbol references */
    volatile int *addrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_default_vis,
        &tls_used,
        &tls_dllimport,
        &tls_uninitialized
    };
    
    /* Use the addresses to prevent optimization */
    int sum = 0;
    for (int i = 0; i < sizeof(addrs)/sizeof(addrs[0]); i++) {
        sum += *addrs[i];
    }
    
    /* Store result in a TLS variable */
    tls_uninitialized = sum;
}

int main(void) {
    /* Initialize uninitialized TLS variable */
    tls_uninitialized = 100;
    
    /* Use all TLS variables in main */
    int result = tls_default + tls_static + tls_extern + tls_weak;
    result += tls_hidden + tls_default_vis + tls_used + tls_dllimport;
    result += tls_uninitialized;
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS variables in helper function */
    modify_tls();
    
    /* Recalculate after modification */
    result = tls_default + tls_static + tls_extern + tls_weak;
    result += tls_hidden + tls_default_vis + tls_used + tls_dllimport;
    result += tls_uninitialized;
    
    printf("After modify_tls: %d\n", result);
    
    /* Take addresses of all TLS variables */
    take_tls_addresses();
    
    /* Final calculation */
    result = tls_default + tls_static + tls_extern + tls_weak;
    result += tls_hidden + tls_default_vis + tls_used + tls_dllimport;
    result += tls_uninitialized;
    
    printf("Final sum: %d\n", result);
    
    /* Return something based on TLS values */
    return (result > 0) ? 0 : 1;
}
