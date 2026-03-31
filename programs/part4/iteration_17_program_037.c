#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will trigger declaration merging
extern __thread int tls_extern_var;

// Function prototype
void tls_operations(int value);

// Conditional compilation for path variation
#ifdef USE_WEAK_TLS
extern __thread int weak_tls_var __attribute__((weak));
#else
extern __thread int strong_tls_var;
#endif

#ifdef USE_HIDDEN
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
#endif

#endif
