/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Common TLS (uninitialized) - will become DECL_COMMON */
__thread int common_tls;

/* DLL import simulation (for DECL_DLLIMPORT_P coverage) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden but externally accessible */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function to get address of TLS variable (forces address-taking) */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function using weak TLS */
int use_weak_tls(void) {
    return weak_tls_var * 2;
}

/* Initialize common TLS */
void init_common_tls(int value) {
    common_tls = value;
}

/* Function context for static TLS */
static void helper_function(void) {
    /* Static TLS inside function context - tests DECL_CONTEXT copying */
    static __thread int function_local_tls = 999;
    
    /* Use it to ensure TREE_USED is set */
    function_local_tls++;
}

void call_helper(void) {
    helper_function();
}
