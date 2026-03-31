/* Declaration of TLS variable with various attributes.
   This file declares the variable as external with weak linkage. */

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* Declare TLS variable with:
   - extern (DECL_EXTERNAL)
   - weak (DECL_WEAK) 
   - visibility attribute (DECL_VISIBILITY_SPECIFIED)
   - dllimport on Windows (DECL_DLLIMPORT_P) */
extern DLL_IMPORT __attribute__((weak, visibility("hidden"))) 
__thread int emulated_tls_var;

/* Function prototype */
int get_tls_value(void);
