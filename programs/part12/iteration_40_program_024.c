#ifndef COMMON_H
#define COMMON_H

#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT
    #define DLL_EXPORT
#endif

// Extern TLS variable - declared in header, defined in helper1.c
extern __thread int extern_tls;

// Weak TLS variable - weak linkage
extern __thread int weak_tls_var __attribute__((weak));

// TLS variable with hidden visibility
extern __thread int hidden_tls __attribute__((visibility("hidden")));

// Common TLS variable - tentative definition
extern __thread int common_tls;

// DLL import/export scenario
#ifdef _WIN32
extern DLL_IMPORT __thread int dll_tls_var;
#else
extern __thread int dll_tls_var;
#endif

// Function declarations
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

#endif
