/* This should trigger emulated TLS code generation */
/* Test case for tree-emutls.cc attribute copying logic */

#include <stdio.h>

/* Force emulated TLS by targeting architectures without native TLS support */
/* or compile with -femulated-tls flag */

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage, initialized */
static __thread int tls_static = 2;

/* 3. External TLS declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol - may be overridden */
__attribute__((weak)) __thread int tls_weak = 4;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* 6. TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible = 6;

/* 7. Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* 8. Uninitialized TLS */
__thread int tls_uninit;

/* 9. DLL import simulation (for Windows-like targets) */
/* Note: dllimport typically requires external declaration */
#ifdef _WIN32
__attribute__((dllimport)) extern __thread int tls_dllimport;
#else
/* Simulate with declspec for cross-compilation testing */
__attribute__((dllimport)) __thread int tls_dllimport_sim = 9;
#endif

/* 10. Common TLS (uninitialized external) - will become DECL_COMMON */
extern __thread int tls_common;

/* Definition of external/declared TLS variables */
__thread int tls_extern = 3;
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_static += tls_default;
    tls_hidden = tls_visible * 2;
    tls_used = tls_weak + 1;
    
    /* Take address to inhibit optimizations */
    int *ptr = &tls_static;
    *ptr += 1;
    
    /* Use uninitialized TLS */
    tls_uninit = 100;
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Taking addresses forces symbol references */
    volatile int *addrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_visible,
        &tls_used,
        &tls_uninit,
        &tls_common
    };
    
    /* Use the addresses to prevent optimization */
    for (int i = 0; i < (int)(sizeof(addrs)/sizeof(addrs[0])); i++) {
        if (addrs[i]) {
            /* Create side effect */
            *(int*)addrs[i] += i;
        }
    }
}

int main(void) {
    int sum = 0;
    
    /* Initialize uninitialized TLS */
    tls_uninit = 8;
    tls_common = 10;
    
    /* Use all TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_used;
    sum += tls_uninit;
    sum += tls_common;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum */
    sum = tls_default + tls_static + tls_extern + tls_weak +
          tls_hidden + tls_visible + tls_used + tls_uninit + tls_common;
    
    printf("After modify_tls: %d\n", sum);
    
    /* Take addresses (forces symbol preservation) */
    take_tls_addresses();
    
    /* Final calculation and output */
    sum = tls_default + tls_static + tls_extern + tls_weak +
          tls_hidden + tls_visible + tls_used + tls_uninit + tls_common;
    
    printf("Final sum: %d\n", sum);
    
    /* Use DLL import simulation if available */
#ifdef _WIN32
    if (&tls_dllimport) {
        sum += tls_dllimport;
    }
#else
    sum += tls_dllimport_sim;
#endif
    
    return sum > 0 ? 0 : 1;
}

/* Additional TLS in different scope for more coverage */
static void nested_function(void) {
    /* Local static TLS */
    static __thread int local_tls = 42;
    local_tls++;
    
    /* Reference external TLS */
    tls_common = local_tls;
}
