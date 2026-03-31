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

/* DLL import attribute (for Windows targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* Simulate similar behavior on non-Windows */
__thread int imported_tls = 0;
#endif

/* Function to initialize and use TLS variables */
void init_tls_vars(int seed) {
    /* Initialize with non-constant values to prevent optimization */
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed * 3;
    common_tls = seed - 1;
    
    /* Use volatile to prevent constant folding */
    volatile int* ptr = &imported_tls;
    *((int*)ptr) = seed * 4;
}

/* Function that takes addresses of TLS variables */
void* get_tls_addresses(void) {
    static void* addrs[5];
    addrs[0] = &public_tls;
    addrs[1] = &static_tls;
    addrs[2] = &weak_tls;
    addrs[3] = &common_tls;
    addrs[4] = &imported_tls;
    
    /* Opaque use of addresses */
    printf("TLS addresses: %p %p %p %p %p\n", 
           addrs[0], addrs[1], addrs[2], addrs[3], addrs[4]);
    
    return addrs[0];
}

/* Function that performs arithmetic on TLS variables */
int compute_tls_sum(void) {
    int sum = public_tls + static_tls + weak_tls + common_tls + imported_tls;
    
    /* Complex enough to prevent optimization */
    sum = (sum * 1103515245 + 12345) & 0x7fffffff;
    
    return sum;
}

#endif /* __GNUC__ */
