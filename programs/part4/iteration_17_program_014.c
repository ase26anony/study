#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will trigger property copying
extern __thread int tls_extern_var __attribute__((visibility("default")));

// Function prototype
void tls_operations(int value);

// Conditional compilation for path variation
#ifdef USE_WEAK_TLS
extern __thread int weak_tls_var __attribute__((weak));
#else
extern __thread int strong_tls_var;
#endif

#ifdef USE_DLLIMPORT
#ifdef _WIN32
__declspec(dllimport) extern __thread int imported_tls_var;
#else
extern __thread int imported_tls_var __attribute__((dllimport));
#endif
#endif

#endif // SHARED_H
