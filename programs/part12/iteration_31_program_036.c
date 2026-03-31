#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern TLS declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_external_var;  // Just extern, no definition in header

// Struct type TLS variable
struct Point { int x; int y; };
extern __thread struct Point tls_struct_var __attribute__((used));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);

// Volatile function to prevent optimization
void volatile_write(int* ptr, int value) __attribute__((noinline));

#endif // TLS_VARS_H
