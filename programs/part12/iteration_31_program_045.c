#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern TLS declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_vis_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;

// Static TLS in header (different linkage context)
static __thread int tls_static_in_header;

// Complex type TLS
struct ComplexStruct {
    int a;
    double b;
    void* c;
};
extern __thread struct ComplexStruct tls_struct_var __attribute__((used));

// DLL import simulation (Windows-specific attribute)
#ifdef _WIN32
extern __thread int tls_dllimport_var __attribute__((dllimport));
#else
extern __thread int tls_dllimport_var;
#endif

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void* get_tls_address(int index, int loop_iter);

#endif // TLS_VARS_H
