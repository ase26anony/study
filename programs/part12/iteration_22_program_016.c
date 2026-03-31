/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside a function context */
static void helper_func(void) {
    static __thread int local_func_tls = 100;
    local_func_tls++;  /* Ensure TREE_USED is set */
}

/* DLL import style attribute (simulated) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
__thread int imported_tls __attribute__((dllimport));
#endif

/* Common TLS (uninitialized) */
__thread int common_tls;

/* Function to take address of TLS variables */
int* get_public_tls_addr(void) {
    helper_func();  /* Use the static TLS */
    return &public_tls;
}

int* get_weak_tls_addr(void) {
    return &weak_tls_var;
}

int* get_common_tls_addr(void) {
    return &common_tls;
}
