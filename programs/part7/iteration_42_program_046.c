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

/* DLL import simulation (for Windows/MinGW) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* Simulate similar behavior on non-Windows */
__thread int imported_tls = 0;
#endif

/* Function to initialize and use TLS variables */
void init_and_use_tls(int seed) {
    /* Initialize with non-constant values to prevent optimization */
    public_tls = seed * 2;
    static_tls = seed + 1;
    
    /* Only initialize weak variable if it's defined */
    if (&weak_tls) {
        weak_tls = seed * 3;
    }
    
    common_tls = seed - 1;
    imported_tls = seed * 4;
    
    /* Use variables in non-trivial ways */
    printf("Addresses: public=%p, static=%p, weak=%p\n", 
           (void*)&public_tls, (void*)&static_tls, (void*)&weak_tls);
    
    /* Perform arithmetic operations */
    int sum = public_tls + static_tls + common_tls + imported_tls;
    if (&weak_tls) {
        sum += weak_tls;
    }
    
    /* Opaque use to prevent optimization */
    volatile int dummy = sum;
    (void)dummy;
}

/* Function that returns checksum of TLS variables */
int get_tls_checksum(void) {
    int sum = public_tls + static_tls + common_tls + imported_tls;
    if (&weak_tls) {
        sum += weak_tls;
    }
    return sum % 1000;
}

#endif /* __GNUC__ */
