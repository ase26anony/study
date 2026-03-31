#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will trigger property copying
extern __thread int tls_extern;

// Function prototype
void modify_tls_values(int increment);

// Conditional compilation for path variation
#ifdef USE_WEAK_ATTRIBUTE
#define WEAK_ATTR __attribute__((weak))
#else
#define WEAK_ATTR
#endif

#ifdef USE_VISIBILITY
#define VISIBILITY_ATTR __attribute__((visibility("default")))
#else
#define VISIBILITY_ATTR
#endif

#ifdef _WIN32
#define DLL_ATTR __declspec(dllimport)
#else
#define DLL_ATTR
#endif

#endif
