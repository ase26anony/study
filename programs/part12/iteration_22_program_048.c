/* tls_def.c - Defines TLS variables with different attributes */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Static TLS inside a function context */
static void helper_func(void) {
    static __thread int local_func_tls = 100;
    /* Use it to ensure TREE_USED is set */
    local_func_tls++;
}

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On non-Windows, use visibility hidden then default to test visibility flags */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses TLS variables */
int* get_public_tls_addr(void) {
    helper_func();  /* Ensure local_func_tls is referenced */
    return &public_tls;
}

int get_weak_tls_value(void) {
    return weak_tls_var;
}

void set_common_tls(int value) {
    common_tls = value;
}
