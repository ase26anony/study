/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS emulation attribute copying logic */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

#include <stdio.h>

/* Force emulated TLS by using appropriate attributes and declarations */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static linkage (internal) */
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
__attribute__((visibility("default"))) __thread int tls_visible_default = 6;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but declare it anyway */
__thread int tls_dllimport = 8;
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
    tls_used = tls_default + tls_static;
    
    /* Take address to inhibit optimizations */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    
    /* Use addresses to create side effects */
    if (addr1 != addr2) {
        tls_visible_default = 100;
    }
}

/* Another helper to take addresses of weak symbols */
void use_weak_tls(void) {
    int *weak_addr = &tls_weak;
    int *weak_uninit_addr = &tls_weak_uninit;
    
    /* Use the addresses to prevent optimization */
    if (weak_addr) {
        tls_weak = *weak_addr + 1;
    }
    
    if (weak_uninit_addr) {
        *weak_uninit_addr = 42;
    }
}

int main(void) {
    /* Initialize uninitialized TLS variables */
    tls_uninit = 9;
    tls_static_uninit = 10;
    
    /* Print initial values */
    printf("Initial values:\n");
    printf("tls_default: %d\n", tls_default);
    printf("tls_static: %d\n", tls_static);
    printf("tls_extern: %d\n", tls_extern);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_visible_default: %d\n", tls_visible_default);
    printf("tls_used: %d\n", tls_used);
    printf("tls_uninit: %d\n", tls_uninit);
    printf("tls_static_uninit: %d\n", tls_static_uninit);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Use weak TLS helper */
    use_weak_tls();
    
    /* Compute a result using all TLS variables */
    int sum = tls_default + tls_static + tls_extern + tls_weak +
              tls_hidden + tls_visible_default + tls_used +
              tls_uninit + tls_static_uninit;
    
    printf("\nAfter modifications:\n");
    printf("tls_default: %d\n", tls_default);
    printf("tls_static: %d\n", tls_static);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_visible_default: %d\n", tls_visible_default);
    printf("tls_used: %d\n", tls_used);
    
    printf("\nSum of all TLS variables: %d\n", sum);
    
    /* Take address of a TLS variable for side effect */
    int *tls_ptr = &tls_default;
    *tls_ptr += 1;
    printf("Final tls_default via pointer: %d\n", tls_default);
    
    return 0;
}
