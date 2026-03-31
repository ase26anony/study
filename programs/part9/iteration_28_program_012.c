/* tls_decl.h - Declarations for TLS emulation test */
#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with various attributes */
extern DLL_IMPORT __thread int tls_var_public 
    __attribute__((weak, visibility("default")));

/* Another TLS variable with hidden visibility */
extern __thread int tls_var_hidden 
    __attribute__((visibility("hidden")));

/* Function context TLS variable */
void use_function_tls(void);

#endif /* TLS_DECL_H */
