#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will have properties copied during emulation
extern __thread int tls_extern __attribute__((visibility("default")));

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

// Function prototype
void modify_tls_values(int increment);

#endif
