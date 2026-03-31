/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread long common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with external declaration that gets defined elsewhere */
extern __thread int imported_tls;
#endif

/* Function that uses static TLS with non-trivial context */
static int helper() {
    return 100;
}

void set_static_tls() {
    /* Static TLS inside function context */
    static __thread int local_func_tls = helper();
    local_func_tls++;
}

/* Get address of public TLS */
int* get_public_tls_addr() {
    return &public_tls;
}

/* Initialize weak TLS */
void init_weak_tls() {
    if (&weak_tls_var != NULL) {
        weak_tls_var = 99;
    }
}

/* Use common TLS */
void use_common_tls() {
    common_tls = 123456789L;
}
