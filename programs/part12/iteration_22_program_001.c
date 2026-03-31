/* Public TLS with default visibility and non-constant initializer */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside function context */
static void helper_func(void) {
    static __thread int local_func_tls = 100;
    local_func_tls++;  /* Ensure TREE_USED is set */
}

/* Function that takes address of TLS variables */
int* get_public_tls_addr(void) {
    helper_func();  /* Use the static TLS */
    return &public_tls;
}

/* Non-constant initializer function */
int compute_init(void) {
    return 123;
}

/* TLS with non-constant initializer */
__thread int dynamic_init_tls = compute_init();

/* Common TLS (tentative definition) */
__thread int common_tls;
