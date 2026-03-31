#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;  // Just extern, no definition here

// DLL import simulation (for DECL_DLLIMPORT_P)
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif
extern __thread int tls_dllimport_var DLL_IMPORT;

// Common variable (for DECL_COMMON)
extern __thread int tls_common_var;

// Public variable
extern __thread int tls_public_var;

// Struct type to test with complex types
struct tls_struct {
    int a;
    double b;
    void* c;
};

extern __thread struct tls_struct tls_struct_var __attribute__((used));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void* get_tls_addresses(void* buffer, size_t* count);

#endif // TLS_VARS_H
