#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer to potentially be common) */
__thread int common_tls;

/* DLL import attribute simulation (works on Windows/MinGW) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, simulate with external declaration */
extern __thread int imported_tls;
#endif

/* Function to initialize and use TLS variables */
void init_tls_vars(int seed) {
    /* Initialize with non-constant values to prevent optimization */
    public_tls = seed * 2;
    static_tls = seed + 1;
    
    /* Only initialize weak variable if it's defined (not null) */
    if (&weak_tls != NULL) {
        weak_tls = seed * 3;
    }
    
    common_tls = seed * 4;
    
    /* Use the variables in non-trivial ways */
    printf("Addresses: public=%p, static=%p, weak=%p, common=%p\n",
           (void*)&public_tls, (void*)&static_tls, 
           (void*)&weak_tls, (void*)&common_tls);
}

/* Function that modifies TLS variables */
int modify_and_sum_tls(void) {
    int sum = 0;
    
    /* Perform arithmetic operations */
    public_tls += 1;
    static_tls *= 2;
    
    if (&weak_tls != NULL) {
        weak_tls -= 3;
    }
    
    common_tls = common_tls / 2 + 1;
    
    /* Aggregate results */
    sum += public_tls;
    sum += static_tls;
    if (&weak_tls != NULL) {
        sum += weak_tls;
    }
    sum += common_tls;
    
    return sum;
}

#endif /* __GNUC__ */
