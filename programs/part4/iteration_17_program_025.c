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

/* Visibility attributes */
#define VIS_DEFAULT __attribute__((visibility("default")))
#define VIS_HIDDEN __attribute__((visibility("hidden")))

/* Conditional compilation flags */
#ifdef USE_WEAK_TLS
    #define WEAK_ATTR __attribute__((weak))
#else
    #define WEAK_ATTR
#endif

#ifdef USE_DLLIMPORT
    #define EXTERN_TLS_ATTR DLL_IMPORT
#else
    #define EXTERN_TLS_ATTR
#endif

/* TLS declarations with various attributes */
extern EXTERN_TLS_ATTR __thread int tls_extern_var;
extern VIS_DEFAULT __thread int tls_visible_var;
extern WEAK_ATTR __thread int tls_weak_var;

/* Function prototype */
int process_tls_values(int iteration);

#endif /* TLS_COMMON_H */
