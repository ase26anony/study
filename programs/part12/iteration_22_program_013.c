/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread long common_tls;

/* DLL import style attribute (simulated) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On non-Windows, use visibility to simulate */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-trivial context */
static int helper() {
    return 123;
}

void set_tls_values() {
    /* Use all TLS variables to ensure TREE_USED is set */
    public_tls = helper();
    if (&weak_tls_var) {
        weak_tls_var = public_tls * 2;
    }
    common_tls = weak_tls_var + 1;
    
    /* Take address to force runtime setup */
    volatile long* ptr = &common_tls;
    *ptr += 1;
}

/* Export function that returns TLS addresses */
int* get_public_tls_addr() {
    return &public_tls;
}

long* get_common_tls_addr() {
    return &common_tls;
}
