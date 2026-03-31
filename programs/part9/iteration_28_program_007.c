/* tls_decl.h - Declarations for TLS variable with various attributes */

#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with multiple attributes */
extern DLL_IMPORT __thread int tls_var 
    __attribute__((weak, visibility("hidden")));

/* Function prototype */
int use_tls(void);

#endif /* TLS_DECL_H */
