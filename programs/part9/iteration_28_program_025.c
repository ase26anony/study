/* Declare the TLS variable with various attributes */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with weak linkage, visibility, and DLL import */
extern DLL_IMPORT __attribute__((weak, visibility("hidden"))) 
__thread int emulated_tls_var;
