/* Define TLS variables with various attributes to be cloned */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread long common_tls;

/* DLL import simulation (using visibility attributes) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with function context */
static void helper_function(void) {
    /* Static TLS inside a function - gives it DECL_CONTEXT */
    static __thread int local_func_tls = 100;
    local_func_tls++;
}

/* Force preservation of this declaration */
__thread int preserved_tls __attribute__((used)) = 999;

/* Get address of public TLS */
int* get_public_tls_addr(void) {
    helper_function();  /* Ensure local_func_tls is referenced */
    return &public_tls;
}

/* Modify weak TLS */
void set_weak_tls(int value) {
    weak_tls_var = value;
}

/* Initialize common TLS */
void init_common_tls(long value) {
    common_tls = value;
}
