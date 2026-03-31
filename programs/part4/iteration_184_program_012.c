/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* 1. Plain TLS variable with external linkage (default visibility) */
__thread int tls_default = 1;

/* 2. Static TLS variable with internal linkage */
static __thread int tls_static = 2;

/* 3. External TLS declaration (simulating header declaration) */
extern __thread int tls_extern;

/* 4. Weak TLS variable - should set DECL_WEAK */
__attribute__((weak)) __thread int tls_weak = 4;

/* 5. TLS variable with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* 6. TLS variable with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible = 6;

/* 7. Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* 8. DLL import attribute (for Windows-like targets) */
/* Note: This may only be effective when targeting Windows or with appropriate flags */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* For non-Windows, we'll still declare it but without dllimport */
__thread int tls_dllimport = 8;
#endif

/* 9. Uninitialized TLS variable */
__thread int tls_uninitialized;

/* 10. Common TLS variable (uninitialized external) */
__thread int tls_common;

/* Definition of the previously declared extern variable */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS variable if available */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    tls_hidden = 50;
    tls_visible = 60;
    tls_used = 70;
    tls_dllimport = 80;
    
    /* Initialize uninitialized TLS */
    tls_uninitialized = 90;
    tls_common = 99;
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Taking addresses ensures symbols are required */
    volatile int *ptr1 = &tls_default;
    volatile int *ptr2 = &tls_static;
    volatile int *ptr3 = &tls_hidden;
    volatile int *ptr4 = &tls_visible;
    
    /* Use the pointers to create side effects */
    if (ptr1 && ptr2 && ptr3 && ptr4) {
        /* This creates a side effect the compiler can't eliminate */
        *ptr1 = *ptr1 + 1;
    }
}

int main(void) {
    int sum = 0;
    
    /* Initial values */
    printf("Initial TLS values:\n");
    printf("tls_default = %d\n", tls_default);
    printf("tls_static = %d\n", tls_static);
    printf("tls_extern = %d\n", tls_extern);
    printf("tls_weak = %d\n", tls_weak);
    printf("tls_hidden = %d\n", tls_hidden);
    printf("tls_visible = %d\n", tls_visible);
    printf("tls_used = %d\n", tls_used);
    printf("tls_dllimport = %d\n", tls_dllimport);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Take addresses to inhibit optimizations */
    take_tls_addresses();
    
    /* Compute sum using TLS variables */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible + tls_used + tls_dllimport +
          tls_uninitialized + tls_common;
    
    printf("\nAfter modification:\n");
    printf("tls_default = %d\n", tls_default);
    printf("tls_static = %d\n", tls_static);
    printf("tls_extern = %d\n", tls_extern);
    printf("tls_weak = %d\n", tls_weak);
    printf("tls_hidden = %d\n", tls_hidden);
    printf("tls_visible = %d\n", tls_visible);
    printf("tls_used = %d\n", tls_used);
    printf("tls_dllimport = %d\n", tls_dllimport);
    printf("tls_uninitialized = %d\n", tls_uninitialized);
    printf("tls_common = %d\n", tls_common);
    
    printf("\nSum of all TLS variables: %d\n", sum);
    
    /* Return value based on TLS computations */
    return (sum > 0) ? 0 : 1;
}
