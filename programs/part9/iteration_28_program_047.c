/* tls_decl.c - External declaration of TLS variable with various attributes */

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with weak linkage, visibility, and DLL import attributes */
extern DLL_IMPORT __thread int emulated_tls_var 
    __attribute__((weak, visibility("hidden")));

/* Function prototype that will use the TLS variable */
int get_tls_value(void);
