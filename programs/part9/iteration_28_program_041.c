/* Declaration file for TLS variable with various attributes */

/* Ensure we're using emulated TLS */
#pragma GCC tls_model emulated

/* External declaration with weak linkage, visibility, and Windows-specific attributes */
#ifdef _WIN32
__declspec(dllimport)
#endif
extern __thread int emulated_tls_var
    __attribute__((weak, visibility("hidden")));

/* Function prototype */
int get_tls_value(void);
