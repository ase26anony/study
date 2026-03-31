/* Declaration file for TLS variable with various attributes */

/* Ensure we're using emulated TLS */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wignored-attributes"
#endif

/* Public external declaration with weak linkage and visibility */
#ifdef _WIN32
__declspec(dllimport)
#endif
extern __thread int emulated_tls_var
#ifdef __GNUC__
    __attribute__((weak, visibility("hidden")))
#endif
    ;

/* Function prototype */
int get_tls_value(void);
