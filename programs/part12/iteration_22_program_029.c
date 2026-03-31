/* Define TLS variables with different attributes to be cloned */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside a function context */
static void helper_func(void) {
    static __thread int local_func_tls = 100;
    local_func_tls++;  /* Ensure TREE_USED is set */
}

/* TLS with non-constant initializer */
int get_init_value(void) { return 999; }
__thread int dynamic_init_tls = get_init_value();

/* Common TLS (uninitialized) */
__thread int common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden then overridden */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses TLS addresses */
int* get_public_tls_addr(void) {
    helper_func();  /* Trigger local TLS usage */
    return &public_tls;
}

int* get_weak_tls_addr(void) {
    return &weak_tls_var;
}

int* get_dynamic_tls_addr(void) {
    return &dynamic_init_tls;
}
