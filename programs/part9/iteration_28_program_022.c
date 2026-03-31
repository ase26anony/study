/* tls_decl.c - Declaration of TLS variable with various attributes */

/* Force emulated TLS even if native is available */
#pragma GCC target("tls-model=emulated")

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with multiple attributes */
extern DLL_IMPORT __thread int tls_var 
    __attribute__((weak, visibility("hidden"), used));

/* Function prototype */
int get_tls_value(void);
