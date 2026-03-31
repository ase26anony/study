#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with various attributes
// These will be defined in tls_def.c

// 1. Used attribute - sets TREE_USED
extern __thread int tls_used_var __attribute__((used));

// 2. Weak attribute - sets DECL_WEAK
extern __thread int tls_weak_var __attribute__((weak));

// 3. Hidden visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

// 4. Default visibility - also sets visibility flags
extern __thread int tls_default_var __attribute__((visibility("default")));

// 5. DLL import simulation (common on Windows)
#ifdef _WIN32
extern __thread int tls_dllimport_var __attribute__((dllimport));
#else
// Simulate with other attributes
extern __thread int tls_dllimport_var __attribute__((weak, visibility("default")));
#endif

// 6. External declaration - sets DECL_EXTERNAL
extern __thread int tls_external_var;

// 7. Common linkage - sets DECL_COMMON
extern __thread int tls_common_var;

// 8. Public variable - sets TREE_PUBLIC
extern __thread int tls_public_var;

// 9. Static TLS variable (different linkage context)
// This one is NOT extern - will have different TREE_PUBLIC/DECL_COMMON values
__thread static int tls_static_var;

// 10. Complex type with multiple attributes
struct ComplexStruct {
    int a;
    double b;
    void* c;
};

extern __thread struct ComplexStruct tls_struct_var 
    __attribute__((used, visibility("hidden")));

// 11. Thread-local with preservation attribute simulation
// DECL_PRESERVE_P is typically set for certain important declarations
extern __thread int tls_preserve_var __attribute__((used, noinline));

// Function declarations
void init_tls_vars(int seed);
unsigned long compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_addresses(int index);

#endif // TLS_VARS_H
