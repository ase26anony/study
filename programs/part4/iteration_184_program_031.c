/* This should trigger emulated TLS code generation */
/* Test case for TLS emulation attribute propagation */

#include <stdio.h>

/* Force emulated TLS handling */
#ifdef __GNUC__
#define EMULATED_TLS __thread
#else
#define EMULATED_TLS _Thread_local
#endif

/* TLS variable with default visibility and external linkage */
EMULATED_TLS int tls_default = 1;

/* Static TLS variable - internal linkage */
static EMULATED_TLS int tls_static = 2;

/* External TLS declaration (simulates header declaration) */
extern EMULATED_TLS int tls_extern;

/* External TLS definition */
EMULATED_TLS int tls_extern = 3;

/* Weak TLS symbol */
__attribute__((weak)) EMULATED_TLS int tls_weak = 4;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) EMULATED_TLS int tls_hidden = 5;

/* TLS with default visibility specified explicitly */
__attribute__((visibility("default"))) EMULATED_TLS int tls_visible = 6;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) EMULATED_TLS int tls_used = 7;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) EMULATED_TLS int tls_dllimport;
#else
/* On non-Windows, we can't truly dllimport, but declare it normally */
EMULATED_TLS int tls_dllimport = 8;
#endif

/* Uninitialized TLS variables with various attributes */
__attribute__((weak)) EMULATED_TLS int tls_weak_uninit;
__attribute__((visibility("hidden"))) EMULATED_TLS int tls_hidden_uninit;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS variable */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Use hidden TLS variable */
    tls_hidden = tls_default + tls_static;
    
    /* Ensure used attribute is honored */
    tls_used++;
}

/* Function that takes address of TLS variables */
void take_addresses(void) {
    /* Taking addresses forces symbol references */
    int *ptr1 = &tls_default;
    int *ptr2 = &tls_static;
    int *ptr3 = &tls_extern;
    int *ptr4 = &tls_weak;
    int *ptr5 = &tls_hidden;
    int *ptr6 = &tls_visible;
    int *ptr7 = &tls_used;
    
    /* Use pointers to create side effects */
    if (ptr1 && ptr2 && ptr3) {
        *ptr1 += 1;
        *ptr2 += 1;
        *ptr3 += 1;
    }
    
    /* Reference uninitialized TLS variables */
    int *ptr8 = &tls_weak_uninit;
    int *ptr9 = &tls_hidden_uninit;
    
    /* Initialize them if they haven't been */
    if (ptr8) *ptr8 = 42;
    if (ptr9) *ptr9 = 43;
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
    printf("tls_visible: %d\n", tls_visible);
    printf("tls_used: %d\n", tls_used);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Take addresses (forces symbol processing) */
    take_addresses();
    
    /* Compute final sum using all TLS variables */
    sum = tls_default + tls_static + tls_extern + 
          tls_weak + tls_hidden + tls_visible + tls_used;
    
    /* Add uninitialized ones if they were initialized */
    if (&tls_weak_uninit) sum += tls_weak_uninit;
    if (&tls_hidden_uninit) sum += tls_hidden_uninit;
    
    printf("Final sum: %d\n", sum);
    
    /* Return something based on TLS values */
    return (sum > 0) ? 0 : 1;
}

/* Dummy function that references DLL import TLS */
void reference_dllimport_tls(void) {
#ifdef _WIN32
    /* Just referencing should trigger DECL_DLLIMPORT_P */
    extern EMULATED_TLS int tls_dllimport;
    if (&tls_dllimport) {
        /* Do nothing, just reference */
    }
#endif
}
