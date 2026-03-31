/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside a function (non-global context) */
static int* get_local_tls_addr(void) {
    static __thread int local_func_tls = 100;
    return &local_func_tls;
}

/* TLS with non-constant initializer */
int get_value(void) { return 999; }
__thread int dynamic_init_tls = 0;  /* Will be initialized at runtime */

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Function that takes address of TLS variables */
int* get_public_tls_addr(void) {
    return &public_tls;
}

int* get_weak_tls_addr(void) {
    weak_tls_var = 123;  /* Ensure it's used */
    return &weak_tls_var;
}

void init_dynamic_tls(void) {
    dynamic_init_tls = get_value();
}

int* get_local_tls(void) {
    return get_local_tls_addr();
}
