#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// External TLS variable declaration
extern __thread int extern_tls_var;

// Weak TLS variable declaration
extern __thread int weak_tls_var __attribute__((weak));

// Common TLS variable (tentative definition)
extern __thread int common_tls;

// Function declarations
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

// DLL import/export simulation (for DECL_DLLIMPORT_P)
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT __attribute__((visibility("default")))
    #define DLL_EXPORT __attribute__((visibility("default")))
#endif

// DLL imported TLS variable simulation
extern DLL_IMPORT __thread int dll_imported_tls;

#endif // TLS_COMMON_H
