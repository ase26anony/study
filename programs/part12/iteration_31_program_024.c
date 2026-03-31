#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern TLS declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_vis_var __attribute__((visibility("default")));
extern __thread int tls_extern_only;  // DECL_EXTERNAL should be set

// Struct type for complex TLS
struct tls_struct {
    int a;
    double b;
    void* c;
};

// More complex TLS variables
extern __thread struct tls_struct tls_complex_var __attribute__((used));
extern __thread double tls_double_var __attribute__((weak, visibility("hidden")));

// Function declarations
void init_tls_vars(void);
size_t compute_tls_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_addresses(int idx);

#endif // TLS_VARS_H
