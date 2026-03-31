/* test-emutls-attributes.c
 * This should trigger emulated TLS code generation
 * Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC
 */

#include <stdio.h>

/* Force declaration attributes to be set on TLS variables */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static (internal) linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* External definition */
__thread int tls_extern = 3;

/* Weak symbol */
__attribute__((weak)) __thread int tls_weak;

/* Hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* Default visibility explicitly */
__attribute__((visibility("default"))) __thread int tls_visible_default;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used_attr;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but we'll declare it anyway */
__thread int tls_dllimport;
#endif

/* Common symbol (uninitialized external) */
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Initialize uninitialized TLS */
    if (tls_weak == 0) {
        tls_weak = 42;
    }
    
    tls_hidden = 100;
    tls_visible_default = 200;
    tls_used_attr = 300;
    tls_common = 400;
    
    /* Take address to inhibit optimizations */
    int *ptr = &tls_hidden;
    *ptr += 1;
}

/* Another helper to take addresses of TLS variables */
void take_tls_addresses(void) {
    /* Taking addresses forces symbol references */
    volatile int *p1 = &tls_default;
    volatile int *p2 = &tls_static;
    volatile int *p3 = &tls_extern;
    volatile int *p4 = &tls_weak;
    volatile int *p5 = &tls_hidden;
    volatile int *p6 = &tls_visible_default;
    volatile int *p7 = &tls_used_attr;
    volatile int *p8 = &tls_common;
    
    /* Use pointers to create side effects */
    if (p1 && p2 && p3 && p4 && p5 && p6 && p7 && p8) {
        /* Do nothing, just reference them */
    }
}

int main(void) {
    int sum = 0;
    
    /* Initial use of TLS variables */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Calculate new sum */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible_default + tls_used_attr + tls_common;
    
    printf("Modified sum: %d\n", sum);
    
    /* Force address taking */
    take_tls_addresses();
    
    /* Use DLL import variable if available */
#ifdef _WIN32
    extern __thread int tls_dllimport;
    printf("DLL import var address: %p\n", (void*)&tls_dllimport);
#endif
    
    /* Return value based on TLS state to prevent optimization */
    return (sum > 1000) ? 0 : 1;
}

/* Simulate external definition from another compilation unit */
/* This creates DECL_EXTERNAL scenario */
extern __thread int external_tls_var;
__thread int external_tls_var = 999;

/* Additional TLS with complex initializer */
__thread int tls_with_expr = (2 + 3) * 5;

/* TLS in different scopes to test context */
void function_with_local_tls(void) {
    static __thread int local_static_tls = 123;
    local_static_tls++;
}
