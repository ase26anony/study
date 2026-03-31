#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration that will be defined in tls.c
extern __thread int tls_extern_var __attribute__((visibility("default")));

// Function prototype
void tls_operations(int n);

// Conditional compilation for path variation
#ifdef USE_DLLIMPORT
#  ifdef _WIN32
#    define DLL_ATTR __declspec(dllimport)
#  else
#    define DLL_ATTR __attribute__((dllimport))
#  endif
#else
#  define DLL_ATTR
#endif

#ifdef WEAK_TLS
#  define WEAK_ATTR __attribute__((weak))
#else
#  define WEAK_ATTR
#endif

#endif // SHARED_H
