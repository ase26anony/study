#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Declare external TLS variables with various attributes
extern __thread int tls_extern_var __attribute__((visibility("default")));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_dllimport_var;

// Function prototype
void tls_operations(int value);

// Conditional compilation for different configurations
#ifdef USE_STATIC_TLS
static __thread int tls_static_var = 42;
#else
extern __thread int tls_static_var;
#endif

#ifdef HIDDEN_VISIBILITY
__thread int tls_hidden_var __attribute__((visibility("hidden")));
#else
extern __thread int tls_hidden_var;
#endif

#endif // TLS_COMMON_H
