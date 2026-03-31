/* Main test driver for emulated TLS attribute coverage */
#include <stdio.h>
#include <stdint.h>

/* Forward declarations for TLS variables defined in other files */
extern __thread int tls_extern_var;
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_dllimport_var;

/* Global TLS variables with various attributes */
__thread int tls_global = 42;                     /* TREE_PUBLIC, has initializer */
static __thread int tls_static = 100;             /* Not TREE_PUBLIC */
__thread int tls_common;                          /* DECL_COMMON (tentative definition) */
__thread int tls_used __attribute__((used)) = 1;  /* DECL_PRESERVE_P */

/* Weak TLS variable definition */
__thread int tls_weak_var_def = 999 __attribute__((weak));

/* TLS variable with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 50;

/* Function to ensure TLS variables are marked as used */
void use_tls_variables(void) {
    /* Read and modify TLS variables to ensure TREE_USED is set */
    tls_global += 1;
    tls_static *= 2;
    tls_common = tls_global + tls_static;
    tls_used = tls_used * 3;
    
    /* Use weak variable if available */
    if (&tls_weak_var != NULL) {
        tls_common += tls_weak_var;
    }
    
    /* Use hidden TLS variable */
    tls_hidden -= 10;
    
    /* Use extern variable */
    tls_common += tls_extern_var;
    
    /* Use dllimport variable if available */
#ifdef _WIN32
    tls_common += tls_dllimport_var;
#endif
}

/* Another function in different context */
static void static_function(void) {
    /* TLS variable in function scope - different DECL_CONTEXT */
    static __thread int tls_in_function = 77;
    tls_in_function++;
    tls_global += tls_in_function;
}

/* Take address of TLS variables to ensure they're fully processed */
void take_addresses(void) {
    volatile uintptr_t addr;
    
    addr = (uintptr_t)&tls_global;
    addr = (uintptr_t)&tls_static;
    addr = (uintptr_t)&tls_common;
    addr = (uintptr_t)&tls_used;
    addr = (uintptr_t)&tls_hidden;
    
    /* Use asm to prevent optimization */
    __asm__ volatile ("" : : "r"(addr));
}

/* Complex expression using TLS variables */
int complex_tls_expression(void) {
    return tls_global * 2 + tls_static / 3 + 
           (tls_common ? 1 : 0) + tls_used - tls_hidden;
}

int main(void) {
    int result = 0;
    
    /* Initialize common TLS variable */
    tls_common = 200;
    
    /* Use all TLS variables */
    use_tls_variables();
    static_function();
    take_addresses();
    
    /* Get complex expression result */
    result = complex_tls_expression();
    
    /* Print result to prevent dead code elimination */
    printf("TLS test result: %d\n", result);
    printf("tls_global: %d\n", tls_global);
    printf("tls_static: %d\n", tls_static);
    printf("tls_common: %d\n", tls_common);
    printf("tls_used: %d\n", tls_used);
    printf("tls_hidden: %d\n", tls_hidden);
    
    return 0;
}
