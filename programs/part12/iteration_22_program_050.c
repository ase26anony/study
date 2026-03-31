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
/* On non-Windows, use visibility("hidden") for similar effect */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-global context */
static void helper_function(void) {
    /* Static TLS inside a function - gives it DECL_CONTEXT */
    static __thread int local_func_tls = 100;
    local_func_tls++;
}

/* Function that returns address of TLS variable */
int* get_public_tls_addr(void) {
    helper_function();  /* Ensure local_func_tls is referenced */
    return &public_tls;
}

/* Function that modifies weak TLS */
void init_weak_tls(void) {
    if (&weak_tls_var != NULL) {  /* Reference weak symbol */
        weak_tls_var = 99;
    }
}

/* Non-constant initializer function */
int compute_value(void) {
    return 123;
}

/* TLS with non-constant initializer */
__thread int dynamic_init_tls = 123;  /* Will be copied as initial value */

/* Preserve this symbol (simulate DECL_PRESERVE_P) */
__thread int preserved_tls __attribute__((used)) = 456;
