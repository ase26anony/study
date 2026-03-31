/* tls_decl.c - External declaration of TLS variable with various attributes */

/* Force emulated TLS model */
#pragma GCC tls_model emulated

/* Declare the TLS variable with external linkage and various attributes */
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
int get_tls_value(void);
