/* External declaration of TLS variable with weak linkage and visibility */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with weak linkage, visibility, and dllimport */
DLL_IMPORT extern __thread int tls_var __attribute__((weak, visibility("hidden")));

/* Function prototype */
int get_tls_value(void);
