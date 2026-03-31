#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* Extern TLS declaration - will be defined in tls.c */
extern __thread int tls_extern_var;

/* Weak TLS declaration */
extern __thread int tls_weak_var __attribute__((weak));

/* Function prototype */
int process_tls_values(int x);

/* Conditional compilation for different attributes */
#ifdef USE_VISIBILITY
#define VIS_ATTR __attribute__((visibility("default")))
#else
#define VIS_ATTR
#endif

#ifdef USE_DLLIMPORT
#define IMPORT_ATTR __declspec(dllimport)
#else
#define IMPORT_ATTR
#endif

#endif /* TLS_H */
