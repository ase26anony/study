/* Declaration of the TLS variable with external linkage and attributes */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with weak linkage, visibility, and dllimport */
extern DLL_IMPORT __thread int tls_var 
    __attribute__((weak, visibility("hidden")));
