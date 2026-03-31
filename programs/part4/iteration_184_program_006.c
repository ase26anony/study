/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS handling */
#ifdef __GNUC__
#define TLS_EMU __thread
#else
#define TLS_EMU _Thread_local
#endif

/* Declare TLS variables with various attributes to trigger the uncovered lines */

/* Plain TLS with external linkage, initialized */
TLS_EMU int tls_default = 1;

/* Static TLS with internal linkage */
static TLS_EMU int tls_static = 2;

/* External declaration (simulating header) */
extern TLS_EMU int tls_extern;

/* Weak TLS symbol */
__attribute__((weak)) TLS_EMU int tls_weak = 5;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) TLS_EMU int tls_hidden = 6;

/* TLS with default visibility and used attribute */
__attribute__((visibility("default"))) __attribute__((used)) TLS_EMU int tls_visible_used = 7;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) TLS_EMU int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but we'll keep a variable */
TLS_EMU int tls_dllimport = 8;
#endif

/* Uninitialized TLS */
TLS_EMU int tls_uninitialized;

/* Common TLS (uninitialized external) - simulates DECL_COMMON */
extern TLS_EMU int tls_common;

/* Definition of previously declared extern */
TLS_EMU int tls_extern = 3;

/* Definition of common TLS */
TLS_EMU int tls_common = 9;

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
    
    tls_hidden++;
    tls_visible_used = tls_visible_used * 3 + 1;
    
    /* Take address to inhibit optimizations */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    
    /* Create side effect with addresses */
    if (addr1 != addr2) {
        tls_uninitialized = 1;
    }
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Take addresses of all TLS variables to ensure they're fully processed */
    volatile int *addrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_visible_used,
        &tls_dllimport,
        &tls_uninitialized,
        &tls_common
    };
    
    /* Use the addresses to prevent optimization */
    for (int i = 0; i < (int)(sizeof(addrs)/sizeof(addrs[0])); i++) {
        if (addrs[i]) {
            *addrs[i] += i;
        }
    }
}

int main(void) {
    int sum = 0;
    
    /* Initial values */
    printf("Initial TLS values:\n");
    printf("tls_default: %d\n", tls_default);
    printf("tls_static: %d\n", tls_static);
    printf("tls_extern: %d\n", tls_extern);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_visible_used: %d\n", tls_visible_used);
    
    /* Use TLS variables in main */
    sum = tls_default + tls_static + tls_extern;
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Take addresses to ensure symbols are required */
    take_tls_addresses();
    
    /* Compute final result */
    sum = tls_default + tls_static + tls_extern + tls_hidden + tls_visible_used;
    
    printf("Final sum: %d\n", sum);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_uninitialized: %d\n", tls_uninitialized);
    printf("tls_common: %d\n", tls_common);
    
    return sum > 50 ? 0 : 1;
}
