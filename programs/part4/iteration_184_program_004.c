/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS emulation attribute copying logic */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

#include <stdio.h>

/* Force emulated TLS by using appropriate target or flag */

/* TLS variable with default visibility and external linkage */
__thread int tls_default = 1;

/* TLS variable with static (internal) linkage */
static __thread int tls_static = 2;

/* External TLS declaration (simulating header declaration) */
extern __thread int tls_extern;

/* TLS variable with weak attribute */
__attribute__((weak)) __thread int tls_weak = 5;

/* TLS variable with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 6;

/* TLS variable with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_default_vis = 7;

/* TLS variable with used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 8;

/* Uninitialized TLS variables with different attributes */
__thread int tls_uninit;
__attribute__((weak)) __thread int tls_weak_uninit;
static __thread int tls_static_uninit;

/* Definition of the previously declared extern TLS variable */
__thread int tls_extern = 3;

/* Helper function that modifies TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    tls_weak = 100;
    tls_hidden += 20;
    tls_default_vis = 77;
    tls_used += 5;
    
    /* Use uninitialized TLS variables */
    tls_uninit = 42;
    tls_weak_uninit = 99;
    tls_static_uninit = 123;
}

/* Dummy function that takes address of TLS variable */
void use_tls_address(int *addr) {
    /* Prevent optimization */
    static volatile int sink;
    sink = *addr;
}

int main(void) {
    int sum = 0;
    
    /* Initial values */
    printf("Initial tls_default: %d\n", tls_default);
    printf("Initial tls_static: %d\n", tls_static);
    printf("Initial tls_extern: %d\n", tls_extern);
    printf("Initial tls_weak: %d\n", tls_weak);
    printf("Initial tls_hidden: %d\n", tls_hidden);
    
    /* Modify TLS variables in helper function */
    modify_tls();
    
    /* Read modified values */
    sum = tls_default + tls_static + tls_extern + tls_weak + tls_hidden;
    printf("Sum after modify_tls: %d\n", sum);
    
    /* Take addresses of TLS variables to ensure they're fully processed */
    int *addr_default = &tls_default;
    int *addr_static = &tls_static;
    int *addr_extern = &tls_extern;
    int *addr_weak = &tls_weak;
    int *addr_hidden = &tls_hidden;
    
    /* Use addresses to prevent optimization */
    use_tls_address(addr_default);
    use_tls_address(addr_static);
    use_tls_address(addr_extern);
    use_tls_address(addr_weak);
    use_tls_address(addr_hidden);
    
    /* Additional operations on TLS variables */
    tls_default_vis += sum;
    tls_used *= 2;
    
    /* Final computation using all TLS variables */
    int final_result = tls_default + tls_static + tls_extern + 
                      tls_weak + tls_hidden + tls_default_vis + 
                      tls_used + tls_uninit + tls_weak_uninit + 
                      tls_static_uninit;
    
    printf("Final result: %d\n", final_result);
    
    /* Conditional based on TLS variable to create side effect */
    if (tls_default > 0) {
        printf("tls_default is positive\n");
    }
    
    return 0;
}
