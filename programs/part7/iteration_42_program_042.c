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

/* DLL import simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* Simulate similar behavior on non-Windows */
__thread int imported_tls = 0;
#endif

/* Function to initialize TLS variables with non-constant values */
void init_tls_vars(int seed) {
    /* Use volatile to prevent constant propagation */
    volatile int base = seed * 31;
    
    public_tls = base + 1;
    static_tls = base + 2;
    weak_tls = base + 3;
    common_tls = base + 4;
    
#ifndef _WIN32
    imported_tls = base + 5;
#endif
    
    /* Force TREE_USED to be set by using the variables */
    printf("Initialized TLS vars at %p, %p, %p\n", 
           (void*)&public_tls, (void*)&static_tls, (void*)&weak_tls);
}

/* Function that uses TLS variables in non-trivial ways */
int compute_with_tls(int multiplier) {
    int result = 0;
    
    /* Perform arithmetic operations */
    result += public_tls * multiplier;
    result += static_tls * (multiplier + 1);
    result += weak_tls * (multiplier + 2);
    result += common_tls * (multiplier + 3);
    result += imported_tls * (multiplier + 4);
    
    /* Take addresses and use them */
    int* ptrs[] = {&public_tls, &weak_tls, &common_tls, &imported_tls};
    for (int i = 0; i < 4; i++) {
        result += (int)((long)ptrs[i] & 0xFF);
    }
    
    return result;
}

/* External declaration (will be defined in another file) */
extern __thread int external_tls;

/* Function that uses external TLS */
int use_external_tls(void) {
    if (&external_tls != NULL) {
        external_tls = compute_with_tls(7) % 100;
        return external_tls;
    }
    return 0;
}

#endif /* __GNUC__ */
