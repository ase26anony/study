/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside a function context */
static void inner_function(void) {
    static __thread int local_tls = 100;
    local_tls++;
}

/* Function that uses TLS addresses */
int* get_public_tls_addr(void) {
    return &public_tls;
}

void* get_weak_tls_addr(void) {
    return &weak_tls_var;
}

/* Force usage of static TLS */
void use_inner_tls(void) {
    inner_function();
}

/* Non-constant initializer function */
int compute_initial_value(void) {
    return 1234;
}

/* TLS with non-constant initializer */
__thread int dynamic_tls = 0;

void init_dynamic_tls(void) {
    dynamic_tls = compute_initial_value();
}
