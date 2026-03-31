/* This should trigger emulated TLS code generation */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

#include <stdio.h>

/* TLS variables with various attributes to test attribute copying in tree-emutls.cc */

/* Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS with internal linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* External definition */
__thread int tls_extern = 3;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 4;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible = 6;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but declare it anyway */
__thread int tls_dllimport = 8;
#endif

/* Uninitialized TLS variables to test different cases */
__thread int tls_uninit;
static __thread int tls_static_uninit;
__attribute__((weak)) __thread int tls_weak_uninit;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    tls_hidden = tls_visible + tls_used;
    
    /* Use weak TLS */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Use uninitialized TLS */
    tls_uninit = 42;
    tls_static_uninit = tls_default;
    
    /* Take address to inhibit optimizations */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    (void)addr1;
    (void)addr2;
}

/* Another helper to take more addresses */
void take_addresses(void) {
    /* Take addresses of various TLS variables */
    volatile int *addrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_visible,
        &tls_used,
        &tls_uninit,
        &tls_static_uninit,
        &tls_weak_uninit
    };
    
    /* Use the addresses to create side effects */
    for (int i = 0; i < (int)(sizeof(addrs)/sizeof(addrs[0])); i++) {
        if (addrs[i]) {
            *addrs[i] += i;
        }
    }
}

int main(void) {
    int sum = 0;
    
    /* Initial values */
    printf("Initial values:\n");
    printf("tls_default: %d\n", tls_default);
    printf("tls_static: %d\n", tls_static);
    printf("tls_extern: %d\n", tls_extern);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_visible: %d\n", tls_visible);
    printf("tls_used: %d\n", tls_used);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Take addresses to ensure symbols are required */
    take_addresses();
    
    /* Compute sum using TLS variables */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible + tls_used + tls_uninit +
          tls_static_uninit + tls_weak_uninit;
    
    printf("Sum after modifications: %d\n", sum);
    
    /* Additional use of DLL import simulation */
    tls_dllimport = sum % 100;
    printf("tls_dllimport: %d\n", tls_dllimport);
    
    /* Force use of weak uninitialized TLS */
    if (&tls_weak_uninit) {
        tls_weak_uninit = 999;
        printf("tls_weak_uninit: %d\n", tls_weak_uninit);
    }
    
    return 0;
}
