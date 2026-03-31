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
extern __thread int tls_dllimport_var __attribute__((visibility("default")));
#endif

// Common linkage variable
extern __thread int tls_common_var __attribute__((common));

// Struct type TLS variable
struct tls_struct {
    int a;
    double b;
    void* c;
};
extern __thread struct tls_struct tls_struct_var __attribute__((used));

// Function prototypes
void init_tls_vars(void);
size_t compute_tls_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_addresses(void* buffer, size_t* count);

#endif // TLS_VARS_H
