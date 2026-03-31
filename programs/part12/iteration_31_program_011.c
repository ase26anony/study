#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with diverse attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_external_var;  // Just extern, will be defined elsewhere

// Struct type TLS variable
struct tls_struct {
    int a;
    double b;
    void* c;
};

extern __thread struct tls_struct tls_struct_var __attribute__((used));

// Function declarations
void* take_tls_addresses(int idx, int seed);
int compute_tls_checksum(void);
void write_to_tls_vars(int seed);

#endif // TLS_VARS_H
