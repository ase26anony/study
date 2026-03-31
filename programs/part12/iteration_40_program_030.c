#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Declare external TLS variables
extern __thread int extern_tls;
extern __thread int common_tls;

// Declare weak TLS variable
extern __thread int weak_tls_var __attribute__((weak));

// Function declarations
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

// DLL import/export simulation (for Windows targets)
#ifdef _WIN32
    #ifdef BUILDING_DLL
        #define DLL_ATTR __declspec(dllexport)
    #else
        #define DLL_ATTR __declspec(dllimport)
    #endif
#else
    #define DLL_ATTR
#endif

// DLL-style TLS variable (for DECL_DLLIMPORT_P coverage)
extern DLL_ATTR __thread int dllstyle_tls;

#endif
