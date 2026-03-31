/* tls_def.c - Defines TLS variables with various attributes */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Static TLS inside a function context */
static void helper_func(void) {
    static __thread int local_func_tls = 200;
    local_func_tls++;  /* Ensure TREE_USED is set */
}

/* TLS with non-constant initializer via function call */
int get_value(void) { return 999; }
__thread int dynamic_init_tls = get_value();

/* Common TLS (uninitialized) */
__thread int common_tls;

/* DLL import style attribute (simulated with visibility) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
__thread int imported_tls __attribute__((visibility("default")));
#endif

/* Function that uses TLS variables */
int* get_public_tls_addr(void) {
    helper_func();  /* Trigger use of local_func_tls */
    return &public_tls;
}

int get_weak_tls_value(void) {
    return weak_tls_var;
}

void set_common_tls(int val) {
    common_tls = val;
}

int* get_imported_tls_addr(void) {
    return &imported_tls;
}
