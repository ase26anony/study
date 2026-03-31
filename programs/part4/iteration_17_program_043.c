#ifndef TLS_COMMON_H
#define TLS_COMMON_H

/* Declare external TLS variables with various attributes */
extern __thread int tls_extern_var __attribute__((visibility("default")));
extern __thread int tls_weak_var __attribute__((weak));

/* Function prototype */
void tls_operations(int increment);

/* Conditional compilation for different attribute sets */
#ifdef USE_DLLIMPORT
#  ifdef _WIN32
#    define DLL_ATTR __declspec(dllimport)
#  else
#    define DLL_ATTR __attribute__((dllimport))
#  endif
extern DLL_ATTR __thread int tls_dllimport_var;
#endif

#ifdef HIDDEN_VISIBILITY
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
#else
extern __thread int tls_hidden_var __attribute__((visibility("internal")));
#endif

#endif /* TLS_COMMON_H */
