/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside a function context */
static void foo(void) {
    static __thread int local_tls = 100;
    local_tls++;
}

/* Function to get address of TLS variable */
int* get_public_tls_addr(void) {
    foo();  /* Ensure local_tls is used */
    return &public_tls;
}

/* Function to use weak TLS */
int use_weak_tls(void) {
    return weak_tls_var;
}
