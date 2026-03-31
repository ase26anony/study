/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

/* Force emulated TLS by targeting architectures without native TLS support
   or using -femulated-tls flag */

#include <stdio.h>

/* 1. Basic TLS variables with different storage classes */

/* External linkage by default, initialized */
__thread int tls_default = 1;

/* Internal linkage, initialized */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern_decl;

/* External definition */
__thread int tls_extern_def = 3;

/* Uninitialized TLS */
__thread int tls_uninit;

/* 2. TLS variables with various GCC attributes */

/* Weak symbol TLS */
__attribute__((weak)) __thread int tls_weak = 5;

/* Weak uninitialized */
__attribute__((weak)) __thread int tls_weak_uninit;

/* Hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 7;

/* Default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_default_vis = 8;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 9;

/* DLL import attribute (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Simulate dllimport for cross-compilation testing */
__attribute__((dllimport)) __thread int tls_dllimport_sim;
#endif

/* Multiple attributes combined */
__attribute__((weak, visibility("hidden"), used)) 
__thread int tls_multi_attr = 42;

/* 3. Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 1;
    tls_static *= 2;
    
    /* Use weak TLS */
    if (&tls_weak) {
        tls_weak = tls_default + tls_static;
    }
    
    /* Use hidden visibility TLS */
    tls_hidden = tls_default_vis - 1;
    
    /* Ensure used attribute TLS is accessed */
    tls_used = tls_multi_attr;
    
    /* Take address to inhibit optimizations */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    (void)addr1;
    (void)addr2;
}

/* 4. Another function taking TLS address */
int* get_tls_address(void) {
    /* Taking address forces symbol reference */
    return &tls_default_vis;
}

/* 5. Function using common TLS pattern */
void use_common_tls(void) {
    /* Common (uninitialized) TLS access */
    tls_uninit = 100;
    tls_weak_uninit = 200;
}

int main(void) {
    int sum = 0;
    
    /* Initialize extern definition */
    tls_extern_def = 4;
    
    /* Use all TLS variables to prevent elimination */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern_def;
    
    /* Initialize and use uninitialized TLS */
    tls_uninit = 10;
    sum += tls_uninit;
    
    /* Use weak TLS */
    if (&tls_weak) {
        sum += tls_weak;
    }
    
    /* Use visibility-annotated TLS */
    tls_hidden = 20;
    tls_default_vis = 30;
    sum += tls_hidden + tls_default_vis;
    
    /* Use attribute-combined TLS */
    sum += tls_multi_attr;
    
    /* Use 'used' attribute TLS */
    sum += tls_used;
    
    /* Call helper functions */
    modify_tls();
    use_common_tls();
    
    /* Take addresses to force symbol preservation */
    int *addr1 = &tls_default;
    int *addr2 = &tls_static;
    int *addr3 = get_tls_address();
    
    /* Use addresses to create side effects */
    if (addr1 && addr2 && addr3) {
        sum += *addr1 + *addr2 + *addr3;
    }
    
    /* Print result to prevent optimization */
    printf("TLS sum: %d\n", sum);
    printf("tls_default: %d, tls_static: %d\n", tls_default, tls_static);
    printf("tls_weak: %d, tls_hidden: %d\n", tls_weak, tls_hidden);
    
    return 0;
}

/* Definition matching earlier declaration */
__thread int tls_extern_decl = 6;

/* Simulate dllimport definition if not on Windows */
#ifndef _WIN32
__thread int tls_dllimport_sim = 11;
#endif
