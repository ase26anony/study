#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT
    #define DLL_EXPORT
#endif

/* Extern TLS variable - declared in header, defined in helper1.c */
extern __thread int extern_tls_var;

/* Weak TLS variable - may be overridden */
extern __thread int weak_tls_var __attribute__((weak));

/* Common TLS variable - tentative definition */
extern __thread int common_tls;

/* Function declarations */
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

/* DLL-style variable for Windows */
#ifdef _WIN32
    DLL_IMPORT extern __thread int dll_tls_var;
#endif

#endif /* TLS_COMMON_H */
