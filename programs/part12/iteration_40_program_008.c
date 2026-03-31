#ifndef COMMON_H
#define COMMON_H

#ifdef _WIN32
    #ifdef BUILDING_DLL
        #define DLL_ATTR __declspec(dllexport)
    #else
        #define DLL_ATTR __declspec(dllimport)
    #endif
#else
    #define DLL_ATTR
#endif

/* Extern TLS variable - declared in header, defined in helper1.c */
extern __thread int extern_tls;

/* Weak TLS variable - weak linkage */
extern __thread int weak_tls_var __attribute__((weak));

/* Common TLS variable - tentative definition */
extern __thread int common_tls;

/* DLL-style TLS variable */
extern DLL_ATTR __thread int dll_tls_var;

/* Function declarations */
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

#endif
