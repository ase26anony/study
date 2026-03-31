#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will have DECL_EXTERNAL set
extern __thread int tls_extern_var;

// Weak TLS declaration - will have DECL_WEAK set
extern __thread int tls_weak_var __attribute__((weak));

// Function prototype
void tls_operations(int n);

// Conditional compilation for path variation
#ifdef USE_DLLIMPORT
#  ifdef _WIN32
#    define DLL_ATTR __declspec(dllimport)
#  else
#    define DLL_ATTR __attribute__((dllimport))
#  endif
extern DLL_ATTR __thread int tls_dllimport_var;
#else
extern __thread int tls_dllimport_var;
#endif

#endif // SHARED_H
