/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with external linkage */
extern __thread int imported_tls;
#endif

/* Function that uses static TLS with non-global context */
static int helper() {
    static __thread int local_func_tls = 100;
    local_func_tls++;
    return local_func_tls;
}

/* Address-taking function */
int* get_public_tls_addr() {
    return &public_tls;
}

int* get_weak_tls_addr() {
    weak_tls_var = 77;  /* Ensure it's used */
    return &weak_tls_var;
}

int use_local_tls() {
    return helper();
}
