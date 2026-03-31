#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// TLS variable declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_vis_var __attribute__((visibility("default")));
extern __thread int tls_external_var;  // Just extern, no definition in header

// Different types and storage classes
extern __thread double tls_double_var;
extern __thread struct Point {
    int x;
    int y;
} tls_struct_var;

// Static TLS in header (internal linkage)
static __thread int tls_static_internal;

// Weak external with visibility
extern __thread int tls_weak_hidden_var __attribute__((weak, visibility("hidden")));

// DLL import simulation (using visibility attributes)
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((visibility("default")))
#endif

extern DLL_IMPORT __thread int tls_dllimport_var;

// Common linkage test
extern __thread int tls_common_var __attribute__((common));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_address(int index);

#endif // TLS_VARS_H
