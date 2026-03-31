/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (uninitialized, may become common) */
__thread long common_tls;

/* DLL import simulation (for attribute copying) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On non-Windows, simulate with visibility */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-global context */
static int helper(void) {
    static __thread int local_func_tls = 100;
    local_func_tls++;
    return local_func_tls;
}

/* Function returning address of TLS variable */
int* get_public_tls_addr(void) {
    /* Ensure TREE_USED is set */
    public_tls = public_tls + 1;
    return &public_tls;
}

/* Function using weak TLS */
int use_weak_tls(void) {
    if (&weak_tls_var != 0) {
        weak_tls_var = 99;
        return weak_tls_var;
    }
    return 0;
}

/* Function using common TLS */
void init_common_tls(void) {
    common_tls = 123456789L;
}

/* Function using imported TLS */
int use_imported_tls(void) {
    imported_tls = 777;
    return imported_tls;
}
