/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread long common_tls;

/* DLL import style attribute (simulated via visibility) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-trivial context */
static int helper() { return 100; }

static void complex_context() {
    /* Static TLS inside a function - gives it function context */
    static __thread int local_func_tls = helper();
    
    /* Mark as used by taking address */
    int *ptr = &local_func_tls;
    *ptr += 1;
}

/* Export function that uses TLS */
int* get_public_tls_addr() { 
    complex_context();  /* Ensure local_func_tls is considered */
    return &public_tls; 
}

/* Function that initializes weak TLS */
int init_weak_tls() {
    if (&weak_tls_var != NULL) {
        weak_tls_var = 99;
    }
    return weak_tls_var;
}
