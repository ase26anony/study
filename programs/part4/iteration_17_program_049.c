#ifndef TLS_COMMON_H
#define TLS_COMMON_H

/* Thread-local storage declarations with various attributes */

/* Extern declaration - will be defined in tls.c */
extern __thread int tls_extern_var __attribute__((visibility("default")));

/* Weak TLS declaration */
extern __thread int tls_weak_var __attribute__((weak));

/* Function prototype */
void tls_operations(int iterations);

/* Conditional compilation for path variation */
#ifdef USE_DLLIMPORT
#  ifdef _WIN32
#    define DLL_ATTR __declspec(dllimport)
#  else
#    define DLL_ATTR __attribute__((dllimport))
#  endif
extern DLL_ATTR __thread int tls_dllimport_var;
#else
extern __thread int tls_dllimport_var __attribute__((common));
#endif

#endif /* TLS_COMMON_H */
