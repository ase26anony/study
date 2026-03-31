/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Force emulated TLS by targeting architectures without native TLS support
   or using -femulated-tls flag */

/* TLS variable with default external linkage, initialized */
__thread int tls_default = 1;

/* TLS variable with internal linkage */
static __thread int tls_static = 2;

/* External TLS declaration (simulating header) */
extern __thread int tls_extern;

/* External TLS definition */
__thread int tls_extern = 3;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible_default;

/* TLS marked as used to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used_attr;

/* DLL import attribute (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can still test with dllimport if cross-compiling */
__attribute__((dllimport)) __thread int tls_dllimport;
#endif

/* Common TLS variable (uninitialized with external linkage) */
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 3;
    tls_hidden = tls_default + tls_static;
    
    /* Use weak TLS variable */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Use the 'used' TLS variable */
    tls_used_attr = 999;
    
    /* Take address of TLS variable to inhibit optimizations */
    int *ptr = &tls_visible_default;
    *ptr = 42;
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Taking addresses forces the compiler to fully process TLS symbols */
    volatile int *addrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_visible_default,
        &tls_used_attr,
        &tls_common
    };
    
    /* Use the addresses to prevent dead code elimination */
    for (int i = 0; i < (int)(sizeof(addrs)/sizeof(addrs[0])); i++) {
        if (addrs[i]) {
            *addrs[i] += i;
        }
    }
}

int main(void) {
    int sum = 0;
    
    /* Initialize some TLS variables */
    tls_hidden = 5;
    tls_visible_default = 7;
    tls_common = 11;
    
    /* Use TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    
    printf("Initial sum: %d\n", sum);
    
    /* Call helper function to modify TLS */
    modify_tls();
    
    /* Recalculate sum with modified values */
    sum = tls_default + tls_static + tls_extern + tls_hidden;
    printf("Modified sum: %d\n", sum);
    
    /* Take addresses of TLS variables */
    take_tls_addresses();
    
    /* Final computation using all TLS variables */
    int final_result = 
        tls_default + tls_static + tls_extern + 
        tls_hidden + tls_visible_default + tls_used_attr +
        tls_common;
    
    printf("Final result: %d\n", final_result);
    
    /* Use weak symbol conditionally */
    if (&tls_weak) {
        printf("Weak TLS at %p, value: %d\n", 
               (void*)&tls_weak, tls_weak);
    }
    
    return 0;
}
