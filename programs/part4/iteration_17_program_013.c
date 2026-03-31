#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will trigger property copying when defined
extern __thread int tls_extern_var __attribute__((visibility("default")));

// Conditional compilation for path variation
#ifdef USE_WEAK
extern __thread int tls_weak_var __attribute__((weak));
#else
extern __thread int tls_strong_var;
#endif

// Function prototype
void tls_operations(int increment);

#endif
