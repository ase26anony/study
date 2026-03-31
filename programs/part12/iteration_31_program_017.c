#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern TLS declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;  // Just extern, no definition in header

// DLL import simulation (Windows-specific attribute)
#ifdef _WIN32
extern __thread int tls_dllimport_var __attribute__((dllimport));
#else
extern __thread int tls_dllimport_var __attribute__((weak));
#endif

// Different types
extern __thread double tls_double_var __attribute__((used));
extern __thread struct Point {
    int x;
    int y;
} tls_struct_var __attribute__((visibility("hidden")));

// Function declarations
void init_tls_vars(void);
size_t compute_tls_checksum(void);
void modify_tls_vars(int seed);

// Helper functions to prevent optimization
void* __attribute__((noinline)) take_address(void* var);
void __attribute__((noinline)) write_to_var(void* var, int value);
int __attribute__((noinline)) read_from_var(void* var);

#endif // TLS_VARS_H
