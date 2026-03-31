#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern declaration with various attributes
extern __thread int tls_extern_used __attribute__((used));
extern __thread int tls_extern_weak __attribute__((weak));
extern __thread int tls_extern_hidden __attribute__((visibility("hidden")));
extern __thread int tls_extern_default __attribute__((visibility("default")));
extern __thread int tls_extern_common;  // Should set DECL_COMMON

// Struct to test complex types
struct tls_struct {
    int a;
    double b;
    void* c;
};

// Extern struct with attributes
extern __thread struct tls_struct tls_struct_extern __attribute__((used, weak));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);

#endif // TLS_VARS_H
