/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside function context */
static void inner_function(void) {
    static __thread int local_context_tls = 100;
    local_context_tls++;
}

/* Function to get address of TLS variable */
int* get_public_tls_addr(void) {
    /* Force TREE_USED to be set */
    public_tls++;
    return &public_tls;
}

/* Function to use weak TLS */
void use_weak_tls(void) {
    if (&weak_tls_var != 0) {
        weak_tls_var = 99;
    }
}

/* Function with static TLS in context */
void use_context_tls(void) {
    inner_function();
}

/* DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* For non-Windows, use visibility attributes */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif
