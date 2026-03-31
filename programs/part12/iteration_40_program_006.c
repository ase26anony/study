#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT
    #define DLL_EXPORT
#endif

// Declare extern TLS variable (will be defined in helper1.c)
extern __thread int extern_tls;

// Declare weak TLS variable
extern __thread int weak_tls_var __attribute__((weak));

// Common TLS variable with multiple definitions
extern __thread int common_tls;

// Hidden visibility TLS variable
extern __thread int hidden_tls __attribute__((visibility("hidden")));

// DLL import/export TLS variable
extern DLL_IMPORT __thread int dll_tls_var;

// Function prototypes
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

#endif
