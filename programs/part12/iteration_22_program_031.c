/* tls_def.c - Defines TLS variables with various linkage and visibility attributes */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate similar concept with visibility */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that returns address of TLS variable */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls(void) {
    if (&weak_tls_var) {
        weak_tls_var = 100;
        return weak_tls_var;
    }
    return 0;
}

/* Function with static TLS in local scope */
void func_with_local_tls(void) {
    /* Static TLS inside function - gives it a DECL_CONTEXT */
    static __thread int local_func_tls = 5;
    local_func_tls++;
}

/* Force preservation by taking address */
int* get_common_tls_addr(void) {
    return &common_tls;
}
