#ifndef COMMON_H
#define COMMON_H

// Declare extern TLS variable
extern __thread int extern_tls;

// Declare weak TLS variable
extern __thread int weak_tls_var __attribute__((weak));

// Declare TLS variable with hidden visibility
extern __thread int hidden_tls __attribute__((visibility("hidden")));

// Common TLS variable (tentative definition)
extern __thread int common_tls;

// Function declarations
void helper1_func(void);
void helper2_func(void);
int compute_checksum(void);

#endif
