/* tls_decl.c - External TLS declarations with various attributes */

/* For Windows DLL import support */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with weak linkage, visibility, and DLL import */
extern DLL_IMPORT __thread int emulated_tls_var __attribute__((weak, visibility("hidden")));

/* Another TLS variable for common linkage testing */
extern __thread int common_tls_var __attribute__((common));

/* Function prototype that will use the TLS variables */
void use_tls_variables(void);
