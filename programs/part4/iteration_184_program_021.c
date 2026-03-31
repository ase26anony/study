/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS emulation attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS by using appropriate compiler flags:
   -femulated-tls or -ftls-model=emulated
   Target architecture without native TLS support (e.g., -march=armv7-a)
*/

/* Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS with internal linkage */
static __thread int tls_static = 2;

/* External TLS declaration (simulating header) */
extern __thread int tls_extern;

/* External TLS definition */
__thread int tls_extern = 3;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* TLS with default visibility and used attribute */
__attribute__((visibility("default"))) __attribute__((used)) __thread int tls_visible_used;

/* DLL import simulation for Windows-like targets */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* For non-Windows, use dllimport-like attribute if supported */
__attribute__((weak)) __thread int tls_dllimport;
#endif

/* Common TLS (uninitialized with external linkage) */
__thread int tls_common;

/* Function that modifies TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 3;
    tls_extern -= 5;
    
    /* Initialize weak TLS if not already defined elsewhere */
    if (tls_weak == 0) {
        tls_weak = 100;
    }
    
    tls_hidden = 42;
    tls_visible_used = 99;
    tls_common = 77;
    
    /* Take address to inhibit optimizations */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    
    /* Use addresses to create side effects */
    if (addr1 != addr2) {
        tls_static++;
    }
}

/* Another function that uses TLS */
int compute_tls_sum(void) {
    int sum = 0;
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible_used;
    sum += tls_common;
    return sum;
}

int main(void) {
    /* Initialize some TLS variables */
    tls_hidden = 7;
    tls_visible_used = 8;
    tls_common = 9;
    
    /* Call function that modifies TLS */
    modify_tls();
    
    /* Use TLS variables in main */
    int result = tls_default + tls_static + tls_extern;
    printf("Basic sum: %d\n", result);
    
    /* Take address of TLS variable */
    int *tls_ptr = &tls_default;
    *tls_ptr += 1;  /* Modify through pointer */
    
    /* Compute comprehensive sum */
    int total = compute_tls_sum();
    printf("Total TLS sum: %d\n", total);
    
    /* Additional operations to ensure all TLS variables are used */
    tls_weak += total;
    tls_dllimport = total % 10;
    
    /* Print final values to prevent optimization */
    printf("tls_default: %d\n", tls_default);
    printf("tls_static: %d\n", tls_static);
    printf("tls_extern: %d\n", tls_extern);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_visible_used: %d\n", tls_visible_used);
    printf("tls_common: %d\n", tls_common);
    
    return 0;
}
