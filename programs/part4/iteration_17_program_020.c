#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Declare extern TLS variable
extern __thread int tls_extern_var __attribute__((visibility("default")));

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

#endif // TLS_COMMON_H
