#ifndef TLS_COMMON_H
#define TLS_COMMON_H

/* Declare TLS variables with various attributes */
extern __thread int tls_extern_var __attribute__((visibility("default")));

/* Weak TLS declaration */
extern __thread int tls_weak_var __attribute__((weak));

/* DLL import simulation for Windows/MinGW */
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
#else
    #define DLL_IMPORT __attribute__((visibility("default")))
#endif

/* Conditional compilation for path variation */
#ifdef USE_COMMON_ATTR
    #define COMMON_ATTR __attribute__((common))
#else
    #define COMMON_ATTR
#endif

/* Function prototype */
void tls_operations(int modifier);

#endif /* TLS_COMMON_H */
