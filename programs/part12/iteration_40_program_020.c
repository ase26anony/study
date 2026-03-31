#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Common declarations for TLS variables

// Extern TLS variable - declared here, defined in helper1.c
extern __thread int extern_tls;

// Weak TLS variable - weak attribute
extern __thread int weak_tls_var __attribute__((weak));

// Common TLS variable - tentative definition
extern __thread int common_tls;

// Function declarations
void helper1_func(void);
void helper2_func(void);

// Visibility attribute test
#ifdef __GNUC__
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#else
#define HIDDEN_VIS
#endif

// Thread-local with hidden visibility
extern _Thread_local int hidden_tls HIDDEN_VIS;

#endif // TLS_COMMON_H
