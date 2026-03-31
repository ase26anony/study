/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread long common_tls;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden then external reference */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function returning address of TLS variable */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function using weak TLS */
int use_weak_tls(void) {
    return weak_tls_var;
}

/* Initialize common TLS */
void init_common_tls(long value) {
    common_tls = value;
}
