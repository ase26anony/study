/* Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol - tests DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside function - tests DECL_CONTEXT */
static int get_value(void) {
    static __thread int local_func_tls = 100;
    return local_func_tls++;
}

/* Common TLS (tentative definition) - tests DECL_COMMON */
__thread int common_tls;

/* DLL import simulation - tests DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses TLS variables */
int* get_public_tls_addr(void) {
    return &public_tls;
}

int get_local_tls_value(void) {
    return get_value();
}

void init_common_tls(void) {
    common_tls = 999;
}
