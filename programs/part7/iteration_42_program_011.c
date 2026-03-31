#ifdef __GNUC__

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common symbol TLS (no initializer) */
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
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed * 3;
    common_tls = seed - 5;
#ifndef _WIN32
    imported_tls = seed + 10;
#endif
}

/* Function that uses TLS variables and prevents optimization */
int compute_from_tls(void) {
    int result = public_tls;
    result += static_tls * 2;
    result ^= weak_tls;
    result += common_tls;
#ifndef _WIN32
    result -= imported_tls;
#endif
    return result;
}

/* Take addresses of TLS variables to ensure they're marked as used */
void* get_tls_addresses(void) {
    static void* addrs[5];
    addrs[0] = (void*)&public_tls;
    addrs[1] = (void*)&static_tls;
    addrs[2] = (void*)&weak_tls;
    addrs[3] = (void*)&common_tls;
    addrs[4] = (void*)&imported_tls;
    return addrs[0]; /* Return one address to prevent dead store elimination */
}

#endif /* __GNUC__ */
