/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* TLS variable with default external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS variable with internal linkage */
static __thread int tls_static = 2;

/* External TLS declaration (simulating header declaration) */
extern __thread int tls_extern;

/* External TLS definition */
__thread int tls_extern = 3;

/* Weak TLS symbol - may be overridden by another definition */
__attribute__((weak)) __thread int tls_weak = 4;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible = 6;

/* TLS marked as used to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but declare it anyway */
__thread int tls_dllimport = 8;
#endif

/* Common TLS variable (uninitialized, may become common symbol) */
__thread int tls_common;

/* Function that modifies TLS variables */
void modify_tls(void) {
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    tls_weak = 100;
    tls_hidden++;
    tls_visible = 42;
    tls_used = tls_used * 3 + 1;
    
    /* Take address of TLS variable to inhibit optimizations */
    int *addr = &tls_hidden;
    *addr += 5;
}

/* Dummy function that uses TLS variable address */
void use_tls_address(int *addr) {
    if (addr) {
        *addr += 1;
    }
}

int main(void) {
    int result = 0;
    
    /* Initialize uninitialized TLS variables */
    tls_common = 9;
    
    /* Use all TLS variables in main */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden;
    result += tls_visible;
    result += tls_used;
    result += tls_dllimport;
    result += tls_common;
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate result */
    result = tls_default + tls_static + tls_extern + tls_weak + 
             tls_hidden + tls_visible + tls_used + tls_dllimport + tls_common;
    
    printf("Modified sum: %d\n", result);
    
    /* Take address of TLS variable and use it */
    int *tls_ptr = &tls_default;
    use_tls_address(tls_ptr);
    
    /* Final computation using the modified value */
    result = tls_default + tls_static;
    printf("Final tls_default + tls_static: %d\n", result);
    
    /* Ensure all TLS variables are referenced to prevent optimization */
    volatile int keep = tls_extern + tls_weak + tls_hidden + 
                       tls_visible + tls_used + tls_dllimport + tls_common;
    (void)keep;
    
    return 0;
}
