#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will trigger declaration merging
extern __thread int tls_extern;

// Function prototype
void tls_operations(int value);

// Conditional compilation for path variation
#ifdef USE_WEAK
#define WEAK_ATTR __attribute__((weak))
#else
#define WEAK_ATTR
#endif

#ifdef USE_DLLIMPORT
#define DLL_ATTR __declspec(dllimport)
#else
#define DLL_ATTR
#endif

#ifdef HIDDEN_VIS
#define VIS_ATTR __attribute__((visibility("hidden")))
#else
#define VIS_ATTR __attribute__((visibility("default")))
#endif

#endif // SHARED_H
