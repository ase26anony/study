/* Public TLS with default visibility and initializer */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import simulation (for DECL_DLLIMPORT_P coverage) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with external declaration */
extern __thread int imported_tls;
#endif

/* Function that returns address of TLS variable */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls(void) {
    if (&weak_tls_var != 0) {
        return weak_tls_var;
    }
    return 0;
}

/* Static function with local static TLS (for DECL_CONTEXT coverage) */
static void helper_function(void) {
    static __thread int local_static_tls = 100;
    local_static_tls++;
    (void)local_static_tls; /* Mark as used */
}

void call_helper(void) {
    helper_function();
}
