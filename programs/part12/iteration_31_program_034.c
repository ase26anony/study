#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern TLS declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_vis_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;

// Struct TLS variable to test complex types
struct tls_struct {
    int a;
    double b;
    void* c;
};

extern __thread struct tls_struct tls_complex_var __attribute__((used));

// Function declarations for taking addresses
void* get_tls_addresses(size_t idx);
void modify_tls_vars(int seed);
int compute_tls_checksum(void);

#endif // TLS_VARS_H
