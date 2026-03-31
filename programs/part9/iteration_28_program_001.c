/* tls_decl.h - Declarations for TLS variables with various attributes */

#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with weak linkage and visibility */
extern __thread int tls_var_public 
    __attribute__((weak, visibility("default")));

/* DLL import declaration for Windows */
extern DLL_IMPORT __thread int tls_var_imported;

/* Hidden visibility variable */
extern __thread int tls_var_hidden 
    __attribute__((visibility("hidden")));

#endif /* TLS_DECL_H */
