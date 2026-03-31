/* Define TLS variables with various attributes to be cloned */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread long common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden then overridden */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function with static TLS inside */
static void inner_function(void) {
    /* Static TLS with function context */
    static __thread int function_local_tls = 100;
    function_local_tls++;
}

/* Get address of public TLS */
int* get_public_tls_addr(void) {
    inner_function();  /* Ensure function_local_tls is used */
    return &public_tls;
}

/* Initialize weak TLS */
void init_weak_tls(void) {
    weak_tls_var = 99;
}

/* Set common TLS */
void set_common_tls(long val) {
    common_tls = val;
}
