/* tls_defs.c - TLS variable definitions with various attributes */
#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (file-local) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable - may be overridden elsewhere */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer to encourage common symbol behavior) */
__thread int common_tls;

/* DLL import attribute simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, simulate with external declaration */
extern __thread int imported_tls;
#endif

/* Function to initialize TLS variables with non-constant values */
void init_tls_vars(int seed) {
    /* Use seed to make initialization non-constant */
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed * 3;
    common_tls = seed - 1;
    
    /* imported_tls is external, can't initialize here */
}

/* Function that uses TLS variables in non-trivial ways */
int compute_from_tls(void) {
    int result = 0;
    
    /* Perform arithmetic operations */
    result += public_tls * 2;
    result += static_tls + 1;
    result ^= weak_tls;
    result *= (common_tls > 0) ? common_tls : 1;
    
    /* Take address to prevent optimization */
    int *ptr = &public_tls;
    result += (int)((long)ptr % 100);
    
    /* Use in conditional */
    if (static_tls > 10) {
        result += 5;
    }
    
    return result % 1000;
}

/* Another function that modifies TLS variables */
void modify_tls_vars(int increment) {
    public_tls += increment;
    static_tls -= increment;
    weak_tls *= (increment + 1);
    if (common_tls == 0) {
        common_tls = increment;
    }
}

#endif /* __GNUC__ */
