/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside a function (non-global context) */
static void foo(void) {
    static __thread int local_tls = 100;
    local_tls++;
}

/* Function that uses TLS variables */
int* get_public_tls_addr(void) {
    foo();  /* Ensure local_tls is used */
    return &public_tls;
}

int get_weak_tls_value(void) {
    return weak_tls_var;
}

/* Non-constant initializer function */
int init_value(void) {
    return 999;
}

/* TLS with non-constant initializer */
__thread int dynamic_tls = 0;

void init_dynamic_tls(void) {
    dynamic_tls = init_value();
}
