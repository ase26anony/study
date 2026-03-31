#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_external_var;  // Just extern, no definition in header

// DLL import simulation (for DECL_DLLIMPORT_P)
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif
extern DLL_IMPORT __thread int tls_dllimport_var;

// Common linkage test
extern __thread int tls_common_var;

// Static TLS in header (different linkage context)
static __thread int tls_static_in_header;

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);

#endif // TLS_VARS_H
