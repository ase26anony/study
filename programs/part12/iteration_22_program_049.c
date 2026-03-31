/* Define TLS variables with various attributes */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Static TLS inside a function (non-global context) */
static void inner_function(void) {
    static __thread int local_func_tls = 200;
    local_func_tls++;  /* Ensure TREE_USED is set */
}

/* TLS with non-constant initializer */
int get_initial_value(void) { return 999; }
__thread int dynamic_init_tls = 0;  /* Will be initialized at runtime */

/* Function that uses TLS addresses */
int* get_public_tls_addr(void) {
    inner_function();  /* Trigger usage of local_func_tls */
    return &public_tls;
}

int* get_weak_tls_addr(void) {
    return &weak_tls_var;
}

void init_dynamic_tls(void) {
    dynamic_init_tls = get_initial_value();
}
