#ifndef TLS_COMMON_H
#define TLS_COMMON_H

/* Attribute macros for different compilers */
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT __attribute__((dllimport))
    #define DLL_EXPORT __attribute__((dllexport))
#endif

/* Conditional compilation for path variation */
#ifdef USE_WEAK_TLS
    #define WEAK_ATTR __attribute__((weak))
#else
    #define WEAK_ATTR
#endif

#ifdef USE_HIDDEN_VISIBILITY
    #define VISIBILITY_ATTR __attribute__((visibility("hidden")))
#else
    #define VISIBILITY_ATTR __attribute__((visibility("default")))
#endif

/* TLS declarations with different attributes */
extern __thread int tls_extern_var;
extern __thread int tls_weak_var WEAK_ATTR;
extern DLL_IMPORT __thread int tls_dllimport_var;

/* Function prototype */
int process_tls_values(int iteration);

#endif /* TLS_COMMON_H */
