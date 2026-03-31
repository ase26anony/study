/* tls_defs.c - TLS variable definitions with various attributes */
#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer to potentially be common) */
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
    
    /* imported_tls is external, don't initialize here */
}

/* Function that uses TLS variables in non-trivial ways */
int compute_tls_sum(void) {
    int sum = 0;
    
    /* Perform arithmetic operations */
    sum += public_tls * 2;
    sum += static_tls + 1;
    sum += weak_tls / 2;
    sum += common_tls * 3;
    
    /* Take addresses to prevent optimization */
    volatile int *addr1 = &public_tls;
    volatile int *addr2 = &static_tls;
    volatile int *addr3 = &weak_tls;
    volatile int *addr4 = &common_tls;
    
    /* Use addresses to ensure TREE_USED is set */
    (void)addr1;
    (void)addr2;
    (void)addr3;
    (void)addr4;
    
    return sum;
}

/* Function that modifies TLS variables */
void modify_tls_vars(int increment) {
    public_tls += increment;
    static_tls -= increment;
    weak_tls *= (increment > 0 ? increment : 1);
    common_tls = common_tls ^ increment; /* XOR operation */
}

#endif /* __GNUC__ */
