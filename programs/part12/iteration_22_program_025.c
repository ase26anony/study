/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import style attribute (simulated) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On non-Windows, use a visibility attribute that might trigger DECL_DLLIMPORT_P */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-global context */
static int helper() {
    static __thread int local_func_tls = 100;
    local_func_tls++;
    return local_func_tls;
}

/* Function returning address of TLS variable */
int* get_public_tls_addr() {
    /* Ensure TREE_USED is set */
    public_tls = public_tls + 1;
    return &public_tls;
}

/* Function using weak TLS */
int use_weak_tls() {
    if (&weak_tls_var != 0) {
        weak_tls_var = 99;
    }
    return weak_tls_var;
}

/* Function with complex initialization */
int init_value() {
    return 1234;
}

/* TLS with non-constant initializer */
__thread int dynamic_init_tls = 0;

void init_dynamic_tls() {
    dynamic_init_tls = init_value();
}
