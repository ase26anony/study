/* This should trigger emulated TLS code generation */
/* Test case for TLS emulation attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* TLS variable with default visibility and external linkage */
__thread int tls_default = 1;

/* Static TLS variable with internal linkage */
static __thread int tls_static = 2;

/* External TLS declaration (simulating header declaration) */
extern __thread int tls_extern;

/* TLS definition for the extern declaration */
__thread int tls_extern = 3;

/* Weak TLS variable - may be overridden by another definition */
__attribute__((weak)) __thread int tls_weak = 4;

/* TLS variable with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* TLS variable with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_default_vis = 6;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* Uninitialized TLS variable */
__thread int tls_uninit;

/* DLL import attribute (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Simulate dllimport for cross-compilation testing */
__attribute__((weak)) __thread int tls_dllimport = 8;
#endif

/* Helper function to modify TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    if (tls_weak) {
        tls_weak = 100;
    }
    
    tls_hidden = tls_default_vis + 20;
    tls_used = 999;
    
    /* Take address to inhibit optimizations */
    int *addr = &tls_uninit;
    *addr = 42;
}

/* Another helper that takes TLS variable addresses */
void use_tls_addresses(void) {
    /* Take addresses of multiple TLS variables */
    int *p1 = &tls_default;
    int *p2 = &tls_static;
    int *p3 = &tls_extern;
    int *p4 = &tls_hidden;
    
    /* Use the addresses to create side effects */
    if (p1 && p2 && p3 && p4) {
        tls_default_vis = *p1 + *p2;
    }
}

int main(void) {
    int sum = 0;
    
    /* Initialize uninitialized TLS variable */
    tls_uninit = 50;
    
    /* Use all TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default_vis;
    sum += tls_used;
    sum += tls_uninit;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS variables in helper function */
    modify_tls();
    
    /* Recalculate sum after modification */
    sum = tls_default + tls_static + tls_extern + tls_weak +
          tls_hidden + tls_default_vis + tls_used + tls_uninit;
    
    printf("Modified sum: %d\n", sum);
    
    /* Use addresses of TLS variables */
    use_tls_addresses();
    
    /* Final calculation using modified values */
    int final = tls_default + tls_static * 2 + tls_extern;
    printf("Final result: %d\n", final);
    
    /* Check DLL import variable if available */
    if (tls_dllimport) {
        printf("DLL import TLS value: %d\n", tls_dllimport);
    }
    
    return 0;
}
