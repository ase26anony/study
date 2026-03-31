#ifdef __GNUC__

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer) */
__thread int common_tls;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* Function to initialize TLS variables with non-constant values */
void init_tls_vars(int seed) {
    /* Use seed to make initialization non-constant */
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed * 3;
    common_tls = seed - 1;
    
    /* imported_tls might be external, so don't initialize if dllimport */
#ifndef _WIN32
    imported_tls = seed * 4;
#endif
}

/* Function that uses TLS variables in non-trivial ways */
int compute_tls_sum(void) {
    int sum = 0;
    
    /* Take address to prevent optimization */
    int *public_ptr = &public_tls;
    int *static_ptr = &static_tls;
    
    /* Perform arithmetic operations */
    sum += public_tls * 2;
    sum += static_tls + 3;
    sum += weak_tls / 2;
    sum += common_tls * common_tls;
    
#ifndef _WIN32
    sum += imported_tls;
#endif
    
    /* Use the pointers to ensure they're not optimized away */
    sum += *public_ptr;
    sum -= *static_ptr;
    
    return sum;
}

/* Function that modifies TLS variables */
void modify_tls_vars(int increment) {
    public_tls += increment;
    static_tls -= increment;
    weak_tls *= (increment > 0 ? increment : 1);
    common_tls += increment * 2;
    
#ifndef _WIN32
    imported_tls += increment * 3;
#endif
}

#endif /* __GNUC__ */
