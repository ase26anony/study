#ifndef TLS_COMMON_H
#define TLS_COMMON_H

// Declare extern TLS variable
extern __thread int extern_tls;

// Weak TLS declaration
extern __thread int weak_tls_var __attribute__((weak));

// Common TLS variable (tentative definition)
extern __thread int common_tls;

// Function declarations
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

#endif // TLS_COMMON_H
