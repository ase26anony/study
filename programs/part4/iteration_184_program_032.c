/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage, initialized */
static __thread int tls_static = 2;

/* 3. External TLS declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol, uninitialized */
__attribute__((weak)) __thread int tls_weak;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* 6. TLS with default visibility and used attribute */
__attribute__((visibility("default"))) __attribute__((used)) __thread int tls_visible_used = 6;

/* 7. DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but we'll define it anyway */
__thread int tls_dllimport;
#endif

/* 8. Common TLS (uninitialized external) - will be marked DECL_COMMON */
__thread int tls_common;

/* External TLS definition */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    
    /* Use weak TLS if available */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    tls_hidden = tls_default + tls_static;
    tls_visible_used++;
    
    /* Take address to ensure symbol is needed */
    int *ptr = &tls_hidden;
    *ptr += 5;
}

/* Another helper to take addresses of TLS variables */
void take_addresses(void) {
    /* Taking addresses inhibits optimizations and ensures symbols are needed */
    volatile int *p1 = &tls_default;
    volatile int *p2 = &tls_static;
    volatile int *p3 = &tls_extern;
    volatile int *p4 = &tls_weak;
    volatile int *p5 = &tls_hidden;
    volatile int *p6 = &tls_visible_used;
    volatile int *p7 = &tls_dllimport;
    volatile int *p8 = &tls_common;
    
    /* Use pointers to create side effects */
    (void)p1; (void)p2; (void)p3; (void)p4;
    (void)p5; (void)p6; (void)p7; (void)p8;
}

int main(void) {
    int sum = 0;
    
    /* Initialize some TLS variables */
    tls_hidden = 4;
    tls_common = 5;
    tls_dllimport = 7;
    
    /* Use TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    
    /* Check if weak symbol is available */
    if (&tls_weak) {
        tls_weak = 8;
        sum += tls_weak;
    }
    
    sum += tls_hidden;
    sum += tls_visible_used;
    sum += tls_dllimport;
    sum += tls_common;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum */
    sum = tls_default + tls_static + tls_extern + tls_hidden + 
          tls_visible_used + tls_dllimport + tls_common;
    
    printf("Modified sum: %d\n", sum);
    
    /* Ensure addresses are taken */
    take_addresses();
    
    /* Return value based on TLS computations */
    return (sum > 0) ? 0 : 1;
}
