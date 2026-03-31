/* External declaration with various attributes */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* Declare TLS variable with multiple attributes */
extern DLL_IMPORT __thread int tls_var 
    __attribute__((weak, visibility("hidden")));
