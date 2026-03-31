#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Extern TLS declaration - will trigger property copying when defined
extern __thread int tls_extern_var __attribute__((visibility("default")));

// Weak TLS declaration
extern __thread int tls_weak_var __attribute__((weak));

// Function prototype
int process_tls_values(int x);

// Macro to conditionally change attributes
#ifdef USE_DLLIMPORT
    #ifdef _WIN32
        #define DLL_ATTR __declspec(dllimport)
    #else
        #define DLL_ATTR __attribute__((dllimport))
    #endif
#else
    #define DLL_ATTR
#endif

#ifdef MAKE_WEAK
    #define WEAK_ATTR __attribute__((weak))
#else
    #define WEAK_ATTR
#endif

#endif // TLS_COMMON_H
