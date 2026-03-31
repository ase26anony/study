#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration - will trigger declaration merging
extern __thread int tls_extern;

// Function prototype from tls.c
void tls_operations(int increment);

// Conditional compilation for path variation
#ifdef USE_WEAK_TLS
extern __thread int tls_weak __attribute__((weak));
#else
extern __thread int tls_strong;
#endif

#ifdef USE_HIDDEN
extern __thread int tls_hidden __attribute__((visibility("hidden")));
#endif

#endif // SHARED_H
