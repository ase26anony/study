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
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif
extern __thread int tls_dllimport_var DLL_IMPORT;

// Different types
extern __thread double tls_double_var __attribute__((used));
extern __thread struct Point {
    int x;
    int y;
} tls_struct_var __attribute__((weak));

// Function prototypes
void init_tls_vars(void);
unsigned long compute_checksum(void);
void modify_tls_vars(int seed);

#endif // TLS_VARS_H
