#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Declare external TLS variables
extern __thread int tls_extern_var;
extern __thread int tls_weak_var __attribute__((weak));

// Function prototype
void tls_operations(int value);

// Conditional compilation macros
#ifdef USE_DLLIMPORT
#  ifdef _WIN32
#    define DLL_ATTR __declspec(dllimport)
#  else
#    define DLL_ATTR __attribute__((dllimport))
#  endif
#else
#  define DLL_ATTR
#endif

#ifdef HIDDEN_VISIBILITY
#  define VIS_ATTR __attribute__((visibility("hidden")))
#else
#  define VIS_ATTR __attribute__((visibility("default")))
#endif

#endif // TLS_COMMON_H
