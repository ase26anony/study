#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer) */
__thread int common_tls;

/* DLL import attribute simulation */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, simulate with external declaration */
extern __thread int imported_tls;
#endif

/* Function to initialize and use TLS variables */
void init_and_use_tls(int base_value) {
    /* Initialize with non-constant values */
    public_tls = base_value + 1;
    static_tls = base_value + 2;
    
    /* Only initialize weak variable if it's defined */
    if (&weak_tls) {
        weak_tls = base_value + 3;
    }
    
    common_tls = base_value + 4;
    
    /* Use variables in non-trivial ways */
    printf("Addresses: public=%p, static=%p, weak=%p, common=%p\n",
           (void*)&public_tls, (void*)&static_tls, 
           (void*)&weak_tls, (void*)&common_tls);
    
    /* Perform arithmetic operations */
    public_tls = public_tls * 2 + static_tls;
    static_tls = static_tls ^ public_tls;
    
    /* Take address and use it */
    int *ptr = &common_tls;
    *ptr += weak_tls;
}

/* Function that returns sum of TLS variables */
int get_tls_sum(void) {
    int sum = public_tls + static_tls;
    
    if (&weak_tls) {
        sum += weak_tls;
    }
    
    sum += common_tls;
    
    /* Try to use imported TLS if available */
    #ifdef _WIN32
    sum += imported_tls;
    #endif
    
    return sum;
}

#endif /* __GNUC__ */
