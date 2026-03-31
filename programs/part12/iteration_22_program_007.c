/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread long common_tls;

/* DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with external declaration */
extern __thread int imported_tls;
#endif

/* Function that uses static TLS with non-global context */
static int helper(void) {
    static __thread int local_func_tls = 100;
    return ++local_func_tls;
}

/* Function returning address of TLS variable */
int* get_public_tls_addr(void) {
    /* Ensure TREE_USED is set */
    public_tls += helper();
    return &public_tls;
}

/* Function using weak TLS */
void use_weak_tls(void) {
    if (&weak_tls_var != 0) {
        weak_tls_var = 99;
    }
}

/* Initialize common TLS */
void init_common_tls(void) {
    common_tls = 123456789L;
}
