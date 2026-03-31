#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer) */
__thread int common_tls;

/* DLL import attribute simulation */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, simulate with external reference */
extern __thread int imported_tls;
#endif

/* Function that uses TLS variables and prevents optimization */
void init_and_use_tls(int seed) {
    /* Initialize with non-constant values */
    public_tls = seed * 2;
    static_tls = seed + 1;
    
    /* Use weak variable if defined */
    if (&weak_tls) {
        weak_tls = seed * 3;
    }
    
    common_tls = seed * 4;
    
    /* Take addresses to prevent optimization */
    volatile int* ptr1 = &public_tls;
    volatile int* ptr2 = &static_tls;
    volatile int* ptr3 = &common_tls;
    
    (void)ptr1;
    (void)ptr2;
    (void)ptr3;
    
    /* Perform arithmetic operations */
    public_tls += static_tls;
    static_tls ^= public_tls;
    
    /* Opaque function call */
    printf("TLS addresses: %p %p %p\n", 
           (void*)&public_tls, 
           (void*)&static_tls,
           (void*)&common_tls);
}

/* Another function that returns TLS-based computation */
int compute_tls_sum(void) {
    int sum = public_tls + static_tls + common_tls;
    
    /* Use weak variable if available */
    if (&weak_tls) {
        sum += weak_tls;
    }
    
    return sum;
}

#endif /* __GNUC__ */
