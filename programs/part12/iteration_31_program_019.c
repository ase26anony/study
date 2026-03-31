#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;  // Just extern, no definition in header

// Struct type to test complex types
struct tls_struct {
    int a;
    double b;
    void* c;
};

// More extern declarations with different types
extern __thread struct tls_struct tls_struct_var __attribute__((used));
extern __thread double tls_double_var __attribute__((weak, visibility("hidden")));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_addresses(void);

#endif // TLS_VARS_H
