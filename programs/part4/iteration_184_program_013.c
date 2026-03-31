/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* TLS variable with default visibility and external linkage */
__thread int tls_default = 1;

/* Static TLS variable - internal linkage */
static __thread int tls_static = 2;

/* External TLS declaration (simulating header declaration) */
extern __thread int tls_extern;

/* External TLS definition */
__thread int tls_extern = 3;

/* Weak TLS symbol - may be overridden */
__attribute__((weak)) __thread int tls_weak = 4;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_default_vis = 6;

/* TLS marked as used to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* Uninitialized TLS variables with different attributes */
__thread int tls_uninit;
__attribute__((weak)) __thread int tls_weak_uninit;
static __thread int tls_static_uninit;

/* For Windows-like environments (cross-compilation scenario) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Simulate similar attribute for non-Windows */
__thread int tls_dllimport = 8;
#endif

/* Helper function that modifies TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS variable */
    if (tls_weak) {
        tls_weak = 100;
    }
    
    /* Modify hidden visibility TLS */
    tls_hidden = tls_default + tls_static;
    
    /* Use the used attribute variable */
    tls_used++;
    
    /* Initialize uninitialized TLS */
    tls_uninit = 42;
    tls_weak_uninit = 43;
    tls_static_uninit = 44;
    
    /* Use dllimport-like variable */
    tls_dllimport = 99;
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Take addresses to inhibit optimizations */
    int *p1 = &tls_default;
    int *p2 = &tls_static;
    int *p3 = &tls_extern;
    int *p4 = &tls_hidden;
    int *p5 = &tls_used;
    
    /* Use pointers to create side effects */
    if (p1 && p2 && p3 && p4 && p5) {
        *p1 += 1;
        *p2 += 1;
    }
    
    /* Also take address of weak symbol */
    int *p6 = &tls_weak;
    if (p6) {
        *p6 = 200;
    }
}

int main(void) {
    int sum = 0;
    
    /* Initial use of TLS variables */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default_vis;
    sum += tls_used;
    sum += tls_dllimport;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum after modification */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_default_vis + tls_used + tls_dllimport +
          tls_uninit + tls_weak_uninit + tls_static_uninit;
    
    printf("Modified sum: %d\n", sum);
    
    /* Take addresses to ensure symbols are required */
    take_tls_addresses();
    
    /* Final calculation with all TLS variables */
    int final_result = 
        tls_default + tls_static * 2 + tls_extern / 2 +
        (tls_weak ? 1 : 0) + tls_hidden % 10 +
        tls_default_vis ^ tls_used + tls_dllimport & 0xFF +
        tls_uninit | tls_weak_uninit + ~tls_static_uninit;
    
    printf("Final result: %d\n", final_result);
    
    /* Return something based on TLS values */
    return (final_result > 0) ? 0 : 1;
}

/* Additional TLS definition for completeness */
__thread int tls_extra = 999;
__attribute__((weak)) __thread int tls_extra_weak;
static __thread int tls_extra_static = 888;
