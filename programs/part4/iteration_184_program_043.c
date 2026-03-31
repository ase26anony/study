/* Test case for tree-emutls.cc attribute copying logic */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* TLS variable with default attributes (external linkage) */
__thread int tls_default = 1;

/* Static TLS variable (internal linkage) */
static __thread int tls_static = 2;

/* External TLS declaration (simulating header declaration) */
extern __thread int tls_extern;

/* External TLS definition */
__thread int tls_extern = 3;

/* Weak TLS variable */
__attribute__((weak)) __thread int tls_weak;

/* TLS variable with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* TLS variable with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible_default;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used_attr;

/* DLL import attribute (simulating Windows cross-compilation) */
/* Note: This may only be valid on certain targets */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* For non-Windows, we'll just declare it normally */
__thread int tls_dllimport;
#endif

/* Common TLS variable (uninitialized, may become common) */
__thread int tls_common;

/* Helper function to modify TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Initialize weak variable if not already defined elsewhere */
    if (tls_weak == 0) {
        tls_weak = 42;
    }
    
    tls_hidden = 100;
    tls_visible_default = 200;
    tls_used_attr = 300;
    tls_dllimport = 400;
    tls_common = 500;
}

/* Dummy function that takes address of TLS variable */
void use_tls_address(int *ptr) {
    /* Prevent optimization */
    volatile int dummy = *ptr;
    (void)dummy;
}

int main(void) {
    int result = 0;
    
    /* Initialize uninitialized TLS variables */
    tls_weak = 5;
    tls_hidden = 6;
    tls_visible_default = 7;
    tls_used_attr = 8;
    tls_dllimport = 9;
    tls_common = 10;
    
    /* Use TLS variables in main */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden;
    result += tls_visible_default;
    result += tls_used_attr;
    result += tls_dllimport;
    result += tls_common;
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS variables in helper function */
    modify_tls();
    
    /* Recalculate sum */
    result = 0;
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden;
    result += tls_visible_default;
    result += tls_used_attr;
    result += tls_dllimport;
    result += tls_common;
    
    printf("Modified sum: %d\n", result);
    
    /* Take addresses of TLS variables to ensure they're fully processed */
    use_tls_address(&tls_default);
    use_tls_address(&tls_static);
    use_tls_address(&tls_extern);
    use_tls_address(&tls_weak);
    use_tls_address(&tls_hidden);
    
    /* Create a side effect with TLS variable addresses */
    volatile int *volatile ptr = &tls_visible_default;
    (void)ptr;
    
    return 0;
}
