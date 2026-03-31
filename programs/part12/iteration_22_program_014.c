/* tls_def.c - Defines TLS variables with different attributes */

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
int get_initial_value(void) { return 999; }
__thread int dynamic_init_tls = get_initial_value();

/* Common TLS (uninitialized global) */
__thread int common_tls;

/* Function that uses TLS variables */
int* get_public_tls_addr(void) {
    helper_func();  /* Reference the static TLS */
    return &public_tls;
}

int get_weak_tls_value(void) {
    return weak_tls_var;
}

void set_common_tls(int value) {
    common_tls = value;
}

int get_dynamic_tls(void) {
    return dynamic_init_tls;
}
