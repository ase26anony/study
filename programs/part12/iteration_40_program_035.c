#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Declare extern TLS variable
extern __thread int extern_tls_var;

// Declare weak TLS variable
extern __thread int weak_tls_var __attribute__((weak));

// Declare common TLS variable (tentative definition)
extern __thread int common_tls;

// Function declarations
void helper1_func(void);
void helper2_func(void);

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

// DLL-style TLS variable declaration
DLL_ATTR extern __thread int dllstyle_tls;

#endif
