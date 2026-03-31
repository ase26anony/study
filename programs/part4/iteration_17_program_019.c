#ifndef SHARED_H
#define SHARED_H

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
int process_tls_values(int iteration);

/* Conditional compilation for different attribute paths */
#ifdef USE_VISIBILITY
#define TLS_VISIBILITY __attribute__((visibility("default")))
#else
#define TLS_VISIBILITY
#endif

#ifdef USE_DLLIMPORT
#define TLS_DLLIMPORT DLL_IMPORT
#else
#define TLS_DLLIMPORT
#endif

#endif /* SHARED_H */
