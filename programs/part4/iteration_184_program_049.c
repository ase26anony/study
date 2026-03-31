/* Test case specifically designed to trigger TLS emulation attribute copying
   in tree-emutls.cc lines 295-304. This should trigger emulated TLS code generation. */

#include <stdio.h>

/* Force emulated TLS by using appropriate attributes and ensuring the variables
   have properties that need to be copied during TLS emulation structure creation. */

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage, initialized */
static __thread int tls_static = 2;

/* 3. External TLS declaration (simulating header declaration) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol - should set DECL_WEAK */
__attribute__((weak)) __thread int tls_weak = 4;

/* 5. TLS with hidden visibility - should set DECL_VISIBILITY */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* 6. TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible = 6;

/* 7. TLS marked as used to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* 8. Uninitialized TLS (common symbol behavior) */
__thread int tls_uninit;

/* 9. External TLS definition (matches declaration above) */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables to ensure they're fully processed */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS variable */
    if (&tls_weak != NULL) {
        tls_weak = 100;
    }
    
    /* Use hidden visibility TLS */
    tls_hidden = tls_default + tls_static;
    
    /* Ensure used attribute is honored */
    tls_used++;
    
    /* Take address of TLS variable to inhibit optimizations */
    int *addr = &tls_visible;
    *addr = 42;
}

/* Dummy function that takes TLS address to create side effects */
void use_tls_address(int *addr) {
    if (addr != NULL) {
        *addr += 1;
    }
}

int main(void) {
    int result = 0;
    
    /* Initialize uninitialized TLS */
    tls_uninit = 8;
    
    /* Use all TLS variables in main */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden;
    result += tls_visible;
    result += tls_used;
    result += tls_uninit;
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate result */
    result = 0;
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden;
    result += tls_visible;
    result += tls_used;
    result += tls_uninit;
    
    printf("Modified sum: %d\n", result);
    
    /* Take address of TLS variable and use it */
    int *tls_addr = &tls_default;
    use_tls_address(tls_addr);
    
    printf("Final tls_default: %d\n", tls_default);
    
    return 0;
}
