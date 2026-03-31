/* Test case for tree-emutls.cc attribute copying logic */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* TLS variable with default external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS variable with internal linkage */
static __thread int tls_static = 2;

/* Extern declaration (simulating header declaration) */
extern __thread int tls_extern;

/* Weak TLS variable - may be overridden */
__attribute__((weak)) __thread int tls_weak = 5;

/* TLS variable with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 6;

/* TLS variable with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible = 7;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 8;

/* DLL import attribute (relevant for Windows targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* For non-Windows, we'll just create another TLS variable */
__thread int tls_regular = 9;
#endif

/* Uninitialized TLS variable */
__thread int tls_uninit;

/* Definition of the extern TLS variable */
__thread int tls_extern = 3;

/* Common TLS variable (simulated by uninitialized external) */
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    if (tls_weak) {
        tls_weak = 100;
    }
    
    tls_hidden = tls_visible;
    tls_used++;
    
    /* Take address to inhibit optimizations */
    int *addr = &tls_hidden;
    *addr += 1;
}

/* Another helper to take addresses of TLS variables */
void take_addresses(void) {
    /* Take addresses of multiple TLS variables */
    int *p1 = &tls_default;
    int *p2 = &tls_static;
    int *p3 = &tls_extern;
    int *p4 = &tls_weak;
    int *p5 = &tls_hidden;
    
    /* Use the addresses to create side effects */
    if (p1 && p2) {
        *p1 = *p2 + 1;
    }
}

int main(void) {
    int sum = 0;
    
    /* Initialize uninitialized TLS variable */
    tls_uninit = 4;
    tls_common = 11;
    
    /* Use all TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_used;
    
#ifdef _WIN32
    if (&tls_dllimport) {
        sum += 1;
    }
#else
    sum += tls_regular;
#endif
    
    sum += tls_uninit;
    sum += tls_common;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS variables in helper function */
    modify_tls();
    
    /* Recalculate sum */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible + tls_used + tls_uninit + tls_common;
    
#ifdef _WIN32
    sum += 1;
#else
    sum += tls_regular;
#endif
    
    printf("Modified sum: %d\n", sum);
    
    /* Take addresses to ensure symbols are needed */
    take_addresses();
    
    /* Final calculation using pointer access */
    int *final_ptr = &tls_default;
    printf("Final value via pointer: %d\n", *final_ptr);
    
    return 0;
}
