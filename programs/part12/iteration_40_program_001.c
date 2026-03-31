#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT
    #define DLL_EXPORT __attribute__((visibility("default")))
#endif

// Declare TLS variables with various attributes
extern __thread int extern_tls;           // External linkage
extern __thread int weak_tls_var __attribute__((weak)); // Weak symbol
extern __thread int common_tls;           // Common symbol (tentative definition)

// Function declarations
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

#endif // TLS_COMMON_H
