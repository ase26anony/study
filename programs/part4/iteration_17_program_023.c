#ifndef SHARED_H
#define SHARED_H

// Extern TLS variable declaration - will be defined in tls.c
extern __thread int tls_extern_var __attribute__((visibility("default")));

// Weak TLS declaration
extern __thread int tls_weak_var __attribute__((weak));

// Function prototype
void tls_operations(int increment);

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

// Conditional TLS variable
#ifdef STATIC_TLS
static __thread int tls_conditional_var;
#else
extern __thread int tls_conditional_var;
#endif

#endif // SHARED_H
