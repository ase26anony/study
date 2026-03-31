#ifdef __GNUC__

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

/* Function to initialize TLS variables with non-constant values */
void init_tls_vars(int seed) {
    /* Use seed to make initialization non-constant */
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed * 3;
    common_tls = seed - 1;
#ifndef _WIN32
    imported_tls = seed * 4;
#endif
}

/* Function that uses TLS variables and returns a value */
int compute_from_tls(void) {
    /* Perform non-trivial operations to ensure TREE_USED is set */
    int result = public_tls;
    result += static_tls * 2;
    result ^= weak_tls;
    result |= common_tls;
#ifndef _WIN32
    result += imported_tls;
#endif
    
    /* Take address to prevent optimization */
    volatile int *addr = &public_tls;
    (void)addr;
    
    return result;
}

/* External declaration that will be defined elsewhere */
extern __thread int external_tls;

#endif /* __GNUC__ */
