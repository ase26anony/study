#ifndef TLS_H
#define TLS_H

// Declare external TLS variables
extern __thread int tls_extern_var __attribute__((visibility("default")));
extern __thread int tls_weak_var __attribute__((weak));

// Function prototype
void tls_operations(int value);

// Conditional compilation macros
#ifdef USE_DLLIMPORT
#define DLL_ATTR __declspec(dllimport)
#else
#define DLL_ATTR
#endif

#ifdef HIDDEN_VISIBILITY
#define VIS_ATTR __attribute__((visibility("hidden")))
#else
#define VIS_ATTR __attribute__((visibility("default")))
#endif

#endif // TLS_H
