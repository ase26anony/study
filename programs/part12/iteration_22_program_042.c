/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with external declaration */
extern __thread int imported_tls;
#endif

/* Function to take address of TLS variables */
int* get_public_tls_addr(void) {
    return &public_tls;
}

int* get_weak_tls_addr(void) {
    return &weak_tls_var;
}

/* Force TREE_USED on these TLS vars */
void mark_tls_used(void) {
    public_tls += 1;
    weak_tls_var += 2;
    common_tls = public_tls + weak_tls_var;
}
