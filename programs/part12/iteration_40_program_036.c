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

// Weak TLS variable - weak definition in helper1.c
extern __thread int weak_tls_var __attribute__((weak));

// Common TLS variable - tentative definitions across files
extern __thread int common_tls;

// Function declarations
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

#endif
