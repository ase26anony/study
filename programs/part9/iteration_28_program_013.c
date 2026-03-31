/* tls_decl.c - Declaration of TLS variable with various attributes */

/* Force emulated TLS */
#pragma GCC tls_model emulated

/* Declare the TLS variable with multiple attributes */
#ifdef _WIN32
__declspec(dllimport)
#endif
extern __thread int emulated_tls_var
#ifdef __GNUC__
    __attribute__((weak))
    __attribute__((visibility("hidden")))
#endif
    ;

/* Function prototype that will use the TLS variable */
int use_tls_variable(void);

/* Another function that might reference the TLS variable */
void indirect_use(void) {
    /* This ensures TREE_USED is set on the declaration */
    if (&emulated_tls_var) {
        /* Just taking address ensures the variable is "used" */
    }
}
