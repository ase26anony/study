/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread int common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate similar concept with visibility */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function with static TLS inside */
static void helper_func(void) {
    /* Static TLS with function context - tests DECL_CONTEXT */
    static __thread int func_local_tls = 100;
    func_local_tls++;
}

/* Get address of public TLS */
int* get_public_tls_addr(void) {
    helper_func();  /* Ensure func_local_tls is referenced */
    return &public_tls;
}

/* Set weak TLS */
void set_weak_tls(int value) {
    weak_tls_var = value;
}

/* Get common TLS address */
int* get_common_tls_addr(void) {
    return &common_tls;
}
