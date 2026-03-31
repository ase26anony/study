/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* TLS with non-constant initializer */
int get_value() { return 100; }
__thread int dynamic_init_tls = get_value();

/* Function to get TLS address */
int* get_public_tls_addr() { 
    return &public_tls; 
}

/* Function using weak TLS */
int* get_weak_tls_addr() { 
    return &weak_tls_var; 
}

/* Static function with local TLS */
static void helper() {
    static __thread int local_static_tls = 5;
    local_static_tls++;
}

void call_helper() {
    helper();
}
