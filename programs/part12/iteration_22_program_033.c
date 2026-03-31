/* Public TLS with default visibility and initializer */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On non-Windows, use visibility("hidden") to test DECL_VISIBILITY */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses TLS and returns its address */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function that modifies weak TLS */
void set_weak_tls(int value) {
    weak_tls_var = value;
}

/* Function with static TLS in local scope (tests DECL_CONTEXT) */
void func_with_local_tls(void) {
    static __thread int local_func_tls = 100;
    local_func_tls++;
}

/* Force TREE_USED by actually using the TLS variables */
void use_all_tls(void) {
    public_tls++;
    weak_tls_var += 2;
    common_tls = public_tls + weak_tls_var;
    func_with_local_tls();
}
