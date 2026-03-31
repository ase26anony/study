#ifndef TLS_COMMON_H
#define TLS_COMMON_H

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
#define TLS_VISIBILITY __attribute__((visibility("default")))
#else
#define TLS_VISIBILITY
#endif

#ifdef USE_DLLIMPORT
#define TLS_DLLIMPORT __declspec(dllimport)
#else
#define TLS_DLLIMPORT
#endif

#endif /* TLS_COMMON_H */
