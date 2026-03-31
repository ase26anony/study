/* Declaration file for TLS variable with various attributes */

/* Ensure we use emulated TLS */
#pragma GCC tls_model emulated

/* Declare the TLS variable with multiple attributes */
#ifdef _WIN32
__declspec(dllimport)
#endif
extern __thread int emulated_tls_var
    __attribute__((weak, visibility("hidden")));

/* Function prototype */
int get_tls_value(void);
