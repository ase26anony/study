#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will trigger declaration merging
extern __thread int tls_extern_var;

// Function prototype from tls.c
void tls_operations(int value);

// Conditional compilation for path variation
#ifdef USE_WEAK_ATTRIBUTE
#define WEAK_ATTR __attribute__((weak))
#else
#define WEAK_ATTR
#endif

#ifdef USE_DLLIMPORT
#ifdef _WIN32
#define DLL_ATTR __declspec(dllimport)
#else
#define DLL_ATTR __attribute__((visibility("default")))
#endif
#else
#define DLL_ATTR
#endif

#endif // SHARED_H
