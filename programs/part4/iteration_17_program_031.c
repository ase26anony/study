#ifndef TLS_COMMON_H
#define TLS_COMMON_H

/* Shared declarations for TLS variables */
extern __thread int tls_extern_var __attribute__((visibility("default")));

/* Function prototype */
int process_tls_values(int x);

/* Conditional compilation flags */
#ifdef USE_WEAK_TLS
    #define WEAK_ATTR __attribute__((weak))
#else
    #define WEAK_ATTR
#endif

#ifdef USE_DLLIMPORT
    #ifdef _WIN32
        #define DLL_ATTR __declspec(dllimport)
    #else
        #define DLL_ATTR __attribute__((dllimport))
    #endif
#else
    #define DLL_ATTR
#endif

#endif /* TLS_COMMON_H */
