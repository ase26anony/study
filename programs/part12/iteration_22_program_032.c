/* tls_def.c - Defines TLS variables with various attributes */

/* Pattern A: Public/Exported TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Pattern B: Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Pattern D: Used TLS variable with non-constant initializer */
int get_initial_value(void) { return 100; }
__thread int init_tls = get_initial_value();

/* Static TLS inside a function context (Pattern C) */
static void foo(void) {
    static __thread int local_tls = 5;
    local_tls++;  /* Ensure it's used */
}

/* Function that takes address of TLS variables */
int* get_public_tls_addr(void) {
    foo();  /* Trigger local_tls usage */
    return &public_tls;
}

int* get_weak_tls_addr(void) {
    return &weak_tls_var;
}

int* get_init_tls_addr(void) {
    return &init_tls;
}

/* Another TLS variable for cross-file reference */
__thread int exported_tls = 10;
