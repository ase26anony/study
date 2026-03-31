/* Define TLS variables with various attributes to be cloned */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread long common_tls;

/* DLL import style attribute (simulated via visibility) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS inside a function context */
static void inner_function(void) {
    /* Static TLS with function context - tests DECL_CONTEXT copying */
    static __thread int function_local_tls = 100;
    function_local_tls++;
}

/* Function that returns address of TLS variable */
int* get_public_tls_addr(void) {
    inner_function();  /* Ensure function_local_tls is used */
    return &public_tls;
}

/* Function that modifies weak TLS */
void set_weak_tls(int value) {
    weak_tls_var = value;
}

/* Non-constant initializer function */
int get_random_value(void) {
    return 12345;  /* Simplified for reproducibility */
}

/* TLS with non-constant initializer */
__thread int dynamic_init_tls = get_random_value();
