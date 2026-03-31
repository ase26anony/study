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

/* Common symbol behavior (no initializer) */
__thread int common_tls;

/* DLL import attribute simulation */
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
    
    /* Only initialize weak variable if it's defined locally */
    if (&weak_tls) {
        weak_tls = seed * 3;
    }
    
    common_tls = seed * 4;
    
    /* Use the variables in non-trivial ways */
    printf("TLS addresses: public=%p, static=%p, weak=%p, common=%p\n",
           (void*)&public_tls, (void*)&static_tls, 
           (void*)&weak_tls, (void*)&common_tls);
    
    /* Perform arithmetic operations */
    public_tls += static_tls;
    weak_tls -= common_tls;
}

/* Function that returns a checksum using TLS variables */
int tls_checksum(void) {
    int sum = 0;
    sum += public_tls;
    sum += static_tls;
    sum += weak_tls;
    sum += common_tls;
    return sum % 1000;
}

#endif /* __GNUC__ */
