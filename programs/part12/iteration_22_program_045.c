/* tls_def.c - Defines TLS variables with different attributes */

/* Public TLS with default visibility and initializer */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate DLL import behavior with visibility */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-global context */
static int helper_function() {
    /* Static TLS inside a function - gives it function context */
    static __thread int local_func_tls = 100;
    local_func_tls++;
    return local_func_tls;
}

/* Function that returns address of TLS variable */
int* get_public_tls_addr() {
    /* Force TREE_USED to be set by using the variable */
    public_tls = public_tls * 2;
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls() {
    if (&weak_tls_var != 0) {  /* Force address taken */
        weak_tls_var = 123;
        return weak_tls_var;
    }
    return 0;
}

/* Function that modifies common TLS */
void init_common_tls() {
    common_tls = 999;
}

/* Function that uses the function-local TLS */
int use_local_tls() {
    return helper_function();
}
