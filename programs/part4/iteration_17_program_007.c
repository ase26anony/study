#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will have DECL_EXTERNAL set
extern __thread int tls_extern;

// Weak TLS declaration
extern __thread int tls_weak __attribute__((weak));

// Function prototype
void modify_tls(int value);

// Conditional compilation for path variation
#ifdef USE_DLLIMPORT
#  ifdef _WIN32
#    define DLL_ATTR __declspec(dllimport)
#  else
#    define DLL_ATTR __attribute__((dllimport))
#  endif
extern DLL_ATTR __thread int tls_dllimport;
#endif

#endif
