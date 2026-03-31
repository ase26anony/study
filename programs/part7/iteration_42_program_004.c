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
    public_tls = seed + 1;
    static_tls = seed + 2;
    weak_tls = seed + 3;
    common_tls = seed + 4;
#ifndef _WIN32
    imported_tls = seed + 5;
#endif
}

/* Function that uses TLS variables in non-trivial ways */
int compute_from_tls(void) {
    int result = 0;
    
    /* Take address to prevent optimization */
    int *ptr1 = &public_tls;
    int *ptr2 = &static_tls;
    
    /* Arithmetic operations */
    result += public_tls * 2;
    result += static_tls * 3;
    result += weak_tls;
    result += common_tls / 2;
    result += imported_tls;
    
    /* Use pointers to force TREE_USED */
    result += *ptr1;
    result += *ptr2;
    
    return result % 1000;
}

/* External declaration (will be defined elsewhere) */
extern __thread int external_tls;

/* Function using external TLS */
void use_external_tls(int val) {
    external_tls = val * 7;
}

#endif /* __GNUC__ */
