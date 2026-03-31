#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Extern TLS declaration - will trigger DECL_EXTERNAL copying
extern __thread int tls_extern_var;

// Attribute macros for different platforms
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT __attribute__((dllimport))
    #define DLL_EXPORT __attribute__((visibility("default")))
#endif

// Function prototype
int process_tls_values(int x);

#endif // TLS_COMMON_H
