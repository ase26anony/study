#ifndef TLS_DECL_H
#define TLS_DECL_H

/* Visibility attribute for DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with various attributes */
extern DLL_IMPORT __thread int emulated_tls_var 
    __attribute__((weak, visibility("hidden")));

/* Function prototype */
int use_tls(void);

#endif /* TLS_DECL_H */
