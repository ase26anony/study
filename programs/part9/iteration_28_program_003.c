/* tls_decl.h - Declarations for TLS variables with various attributes */

#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with weak linkage, visibility, and dllimport */
extern DLL_IMPORT __thread int external_tls_var 
    __attribute__((weak, visibility("default")));

/* Common TLS variable declaration */
extern __thread int common_tls_var __attribute__((common));

#endif /* TLS_DECL_H */
