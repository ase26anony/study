/* Define TLS variables with different attributes to exercise attribute copying */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread long common_tls;

/* DLL import simulation (using visibility attributes) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On Unix-like, use visibility("hidden") for similar effect */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-trivial context */
static int helper() {
    static __thread int local_func_tls = 5;
    local_func_tls++;
    return local_func_tls;
}

/* Function that returns address of TLS variable */
int* get_public_tls_addr() {
    /* Ensure TREE_USED is set by using the variable */
    public_tls += helper();
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls() {
    if (&weak_tls_var != 0) {
        weak_tls_var = 100;
    }
    return weak_tls_var;
}

/* Initialize common TLS with runtime value */
void init_common_tls(int value) {
    common_tls = value * 2;
}

/* Function taking address of imported TLS */
int* get_imported_tls_addr() {
    return &imported_tls;
}
