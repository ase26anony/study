#ifndef TLS_COMMON_H
#define TLS_COMMON_H

/* Attributes to trigger specific declaration properties */
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT __attribute__((dllimport))
    #define DLL_EXPORT __attribute__((dllexport))
#endif

/* Visibility attributes */
#define VIS_DEFAULT __attribute__((visibility("default")))
#define VIS_HIDDEN __attribute__((visibility("hidden")))

/* Extern TLS declaration - will be defined in tls.c */
extern __thread int tls_extern_var VIS_DEFAULT;

/* Weak TLS declaration */
extern __thread int tls_weak_var __attribute__((weak));

/* Function prototype */
int process_tls_values(int iteration);

/* Conditional compilation macro */
#ifdef USE_STATIC_TLS
    #define TLS_STORAGE static __thread
#else
    #define TLS_STORAGE __thread
#endif

#endif /* TLS_COMMON_H */
