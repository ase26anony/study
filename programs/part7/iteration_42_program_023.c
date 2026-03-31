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
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed * 3;
    common_tls = seed - 5;
#ifndef _WIN32
    imported_tls = seed / 2;
#endif
}

/* Function that uses TLS variables non-trivially */
int compute_from_tls(void) {
    int result = 0;
    
    /* Take addresses to prevent optimization */
    int *ptr1 = &public_tls;
    int *ptr2 = &static_tls;
    volatile int *volatile_ptr = &weak_tls;
    
    /* Non-trivial computation */
    result = public_tls + static_tls * 2;
    result += weak_tls * 3;
    result += common_tls;
#ifndef _WIN32
    result += imported_tls * 4;
#endif
    
    /* Use pointers to ensure variables are marked TREE_USED */
    result += *ptr1 + *ptr2;
    
    return result;
}

/* Function that returns addresses for external use */
int* get_public_tls_addr(void) {
    return &public_tls;
}

int* get_weak_tls_addr(void) {
    return &weak_tls;
}

#endif /* __GNUC__ */
