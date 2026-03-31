#ifdef __GNUC__

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread int weak_tls __attribute__((weak)) = 0;

/* Common TLS variable (no initializer to potentially be common) */
__thread int common_tls;

/* Function to initialize and use TLS variables */
void init_tls_values(int base) {
    /* Initialize with non-constant values to prevent optimization */
    public_tls = base + 1;
    static_tls = base + 2;
    
    /* Only initialize weak_tls if it's defined locally */
    #ifndef WEAK_DEFINED_ELSEWHERE
    weak_tls = base + 3;
    #endif
    
    common_tls = base + 4;
}

/* Function that uses TLS variables and returns a checksum */
int compute_tls_checksum(void) {
    int sum = 0;
    
    /* Perform arithmetic operations to ensure usage */
    sum += public_tls * 2;
    sum += static_tls * 3;
    sum += weak_tls * 5;
    sum += common_tls * 7;
    
    /* Take addresses to prevent optimization */
    volatile int *addr1 = &public_tls;
    volatile int *addr2 = &static_tls;
    volatile int *addr3 = &common_tls;
    
    (void)addr1;
    (void)addr2;
    (void)addr3;
    
    return sum % 1000;
}

#endif /* __GNUC__ */
