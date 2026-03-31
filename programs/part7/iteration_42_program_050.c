#ifdef __GNUC__

/* Public TLS variable with initialization */
__thread int public_tls = 123;

/* Static (private) TLS variable */
static __thread int static_tls = 456;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 789;

/* Common symbol behavior (no initializer) */
__thread int common_tls;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* Simulate similar behavior on non-Windows */
__thread int imported_tls = 999;
#endif

/* Function that uses and modifies TLS variables */
int process_tls_values(int seed) {
    /* Use non-constant initialization to prevent optimization */
    public_tls += seed;
    static_tls = seed * 2;
    
    if (&weak_tls) {  /* Take address to ensure it's used */
        weak_tls = seed + 100;
    }
    
    common_tls = seed - 50;
    imported_tls = seed * 3;
    
    /* Perform arithmetic operations */
    int sum = public_tls + static_tls;
    if (&weak_tls) {
        sum += weak_tls;
    }
    sum += common_tls + imported_tls;
    
    return sum;
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

#endif /* __GNUC__ */
