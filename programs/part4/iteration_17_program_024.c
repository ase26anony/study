#ifndef SHARED_H
#define SHARED_H

/* Declare TLS variables with various attributes */
extern __thread int tls_extern_var __attribute__((visibility("default")));
extern __thread int tls_weak_var __attribute__((weak));

/* Function prototype */
void tls_operations(int increment);

/* Conditional compilation for different paths */
#ifdef USE_DLLIMPORT
#  ifdef _WIN32
#    define DLL_ATTR __declspec(dllimport)
#  else
#    define DLL_ATTR __attribute__((dllimport))
#  endif
extern __thread int tls_dllimport_var DLL_ATTR;
#endif

#endif /* SHARED_H */
