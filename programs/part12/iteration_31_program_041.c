#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern TLS declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;  // Just extern, no definition in header

// DLL import simulation (for DECL_DLLIMPORT_P)
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

// Different types
extern __thread double tls_double_var __attribute__((used));
extern __thread struct Point {
    int x;
    int y;
} tls_struct_var __attribute__((visibility("default")));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);

// Volatile function to prevent optimization
void volatile_write(int* ptr, int value) __attribute__((noinline));
int volatile_read(const int* ptr) __attribute__((noinline));

#endif // TLS_VARS_H
