/* This should trigger emulated TLS code generation */
/* Test case for tree-emutls.cc attribute copying logic */

#include <stdio.h>

/* Force emulated TLS handling */
#ifdef __GNUC__
#define TLS __thread
#else
#define TLS _Thread_local
#endif

/* Various TLS declarations with different attributes */

/* 1. Default external linkage, initialized */
TLS int tls_default = 1;

/* 2. Static (internal linkage), initialized */
static TLS int tls_static = 2;

/* 3. External declaration (simulating header) */
extern TLS int tls_extern;

/* 4. Weak symbol */
__attribute__((weak)) TLS int tls_weak = 4;

/* 5. Hidden visibility */
__attribute__((visibility("hidden"))) TLS int tls_hidden = 5;

/* 6. Default visibility explicitly specified */
__attribute__((visibility("default"))) TLS int tls_visible = 6;

/* 7. Used attribute to ensure TREE_USED is set */
__attribute__((used)) TLS int tls_used = 7;

/* 8. DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) TLS int tls_dllimport;
#else
/* Simulate similar attribute on non-Windows */
__attribute__((weak)) __attribute__((visibility("default"))) TLS int tls_dllimport = 8;
#endif

/* 9. Uninitialized TLS */
TLS int tls_uninit;

/* 10. Common linkage simulation */
extern TLS int tls_common;
TLS int tls_common = 9;

/* Definition of the extern declared earlier */
TLS int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden -= 1;
    tls_used = tls_visible + tls_extern;
    
    /* Take address to inhibit optimizations */
    int *addr = &tls_weak;
    *addr += 5;
}

/* Another helper to ensure all TLS variables are referenced */
void reference_all_tls(void) {
    /* Create side effects with all TLS variables */
    volatile int sum = 0;
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_used;
    sum += tls_dllimport;
    sum += tls_uninit;
    sum += tls_common;
    
    /* Use sum to prevent elimination */
    if (sum > 100) {
        printf("TLS reference check passed\n");
    }
}

int main(void) {
    /* Initialize uninitialized TLS */
    tls_uninit = 100;
    
    /* Modify TLS in helper */
    modify_tls();
    
    /* Reference all TLS variables */
    reference_all_tls();
    
    /* Perform computations with TLS variables */
    int result = tls_default + tls_static + tls_extern;
    result += tls_weak + tls_hidden + tls_visible;
    result += tls_used + tls_dllimport + tls_uninit + tls_common;
    
    /* Take addresses to force symbol usage */
    int *ptrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_visible,
        &tls_used,
        &tls_dllimport,
        &tls_uninit,
        &tls_common
    };
    
    /* Use pointers to create side effects */
    for (int i = 0; i < 10; i++) {
        *ptrs[i] += i;
    }
    
    /* Final computation and output */
    int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result += *ptrs[i];
    }
    
    printf("Final TLS sum: %d\n", final_result);
    
    return 0;
}
