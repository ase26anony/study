/* This should trigger emulated TLS code generation */
/* Test case for TLS attribute copying in tree-emutls.cc */

/* Force emulated TLS by using appropriate compilation flags:
   -femulated-tls -O0 -fPIC
   or target ARM without native TLS: -march=armv7-a -ftls-model=emulated
*/

#include <stdio.h>

/* TLS variables with various attributes to test DECL_* attribute copying */

/* Default TLS with external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS with internal linkage */
static __thread int tls_static = 2;

/* Extern declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* TLS with default visibility and used attribute */
__attribute__((visibility("default"), used)) __thread int tls_visible_used;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but we'll declare it anyway */
__thread int tls_dllimport;
#endif

/* Common TLS (uninitialized, external linkage) - tests DECL_COMMON */
__thread int tls_common;

/* Definition of the extern TLS variable */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS if available */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    tls_hidden = 42;
    tls_visible_used = 99;
    tls_common = 77;
    
    /* Take addresses to ensure symbols are needed */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    (void)addr1; (void)addr2; /* Suppress unused warning */
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Taking addresses prevents optimization and ensures TLS symbols are processed */
    volatile int *volatile ptr;
    
    ptr = &tls_default;
    ptr = &tls_static;
    ptr = &tls_extern;
    ptr = &tls_weak;
    ptr = &tls_hidden;
    ptr = &tls_visible_used;
    ptr = &tls_common;
    
    (void)ptr; /* Suppress unused warning */
}

int main(void) {
    int sum = 0;
    
    /* Initialize some TLS variables */
    tls_weak = 5;
    tls_hidden = 7;
    tls_visible_used = 9;
    tls_common = 11;
    
    /* Use TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible_used;
    sum += tls_common;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible_used + tls_common;
    
    printf("Modified sum: %d\n", sum);
    
    /* Ensure addresses are taken */
    take_tls_addresses();
    
    /* Use DLL import TLS if available */
#ifdef _WIN32
    extern __thread int tls_dllimport;
    if (&tls_dllimport) {
        printf("DLL import TLS available\n");
    }
#endif
    
    return 0;
}
