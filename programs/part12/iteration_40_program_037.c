#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT
    #define DLL_EXPORT __attribute__((visibility("default")))
#endif

// External TLS variable - declared extern in header, defined in helper1.c
extern __thread int extern_tls_var;

// Weak TLS variable - weak linkage
extern __thread int weak_tls_var __attribute__((weak));

// Common TLS variable - tentative definition
__thread int common_tls;

// DLL imported TLS variable simulation
#ifdef _WIN32
extern __thread int dll_tls_var DLL_IMPORT;
#else
extern __thread int dll_tls_var __attribute__((visibility("default")));
#endif

// Function declarations
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

#endif
