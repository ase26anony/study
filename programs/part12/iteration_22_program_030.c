/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import style attribute (simulated via dllimport-like attribute) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On non-Windows, use a visibility attribute that might trigger DECL_DLLIMPORT_P */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-trivial context */
static int helper(void) {
    /* Static TLS inside a function - gives it function context */
    static __thread int local_func_tls = 100;
    return ++local_func_tls;
}

/* Function that returns address of TLS variable */
int* get_public_tls_addr(void) {
    /* Ensure TREE_USED is set by taking address */
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls(void) {
    if (&weak_tls_var != 0) {  /* Reference to ensure it's used */
        return weak_tls_var;
    }
    return 0;
}

/* Function with complex TLS usage pattern */
int complex_tls_usage(void) {
    int sum = 0;
    
    /* Multiple uses to ensure TREE_USED is set */
    public_tls += 1;
    sum += public_tls;
    
    common_tls = public_tls * 2;
    sum += common_tls;
    
    sum += helper();
    
    return sum;
}
