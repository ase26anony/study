/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside function context */
static void foo(void) {
    static __thread int local_tls = 100;
    local_tls++;
}

/* Function that returns address of TLS variable */
int* get_public_tls_addr(void) {
    foo();  /* Ensure local_tls is used */
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls(void) {
    if (&weak_tls_var != 0) {
        return weak_tls_var;
    }
    return -1;
}

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On non-Windows, simulate with external declaration */
extern __thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Common TLS (uninitialized, may become common) */
__thread int common_tls;
