/* Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol - tests DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS - tests DECL_COMMON */
__thread int common_tls_var;

/* DLL import simulation - tests DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden but externally available */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that returns address of TLS variable */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function using weak TLS */
int use_weak_tls(void) {
    if (&weak_tls_var) {
        weak_tls_var = 100;
        return weak_tls_var;
    }
    return 0;
}

/* Initialize common TLS */
void init_common_tls(void) {
    common_tls_var = 999;
}
