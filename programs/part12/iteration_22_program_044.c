/* Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol - tests DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS - tests DECL_COMMON */
__thread int common_tls;

/* DLL import style (simulated) - tests DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate similar concept with visibility */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-global context - tests DECL_CONTEXT */
static void helper_function(void) {
    /* Static TLS inside function - has function as DECL_CONTEXT */
    static __thread int local_func_tls = 100;
    local_func_tls++;
}

/* Function to take address of TLS variables - forces address computation */
int* get_public_tls_addr(void) {
    helper_function();  /* Ensure local_func_tls is used */
    return &public_tls;
}

int* get_weak_tls_addr(void) {
    return &weak_tls_var;
}

int* get_common_tls_addr(void) {
    return &common_tls;
}

/* Non-constant initializer function */
int compute_value(void) {
    return 1234;
}

/* TLS with non-constant initializer */
__thread int dynamic_init_tls = 123; /* Will be adjusted at runtime */
