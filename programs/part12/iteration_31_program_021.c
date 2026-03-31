#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with various attributes in header
// These will be extern declarations that need cloning in other compilation units

// Used attribute sets TREE_USED
extern __thread int tls_used_var __attribute__((used));

// Weak attribute sets DECL_WEAK
extern __thread int tls_weak_var __attribute__((weak));

// Hidden visibility sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

// Default visibility
extern __thread int tls_default_var __attribute__((visibility("default")));

// DLL import simulation (DECL_DLLIMPORT_P)
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

// Common linkage variable (DECL_COMMON)
extern __thread int tls_common_var;

// External only (DECL_EXTERNAL)
extern __thread int tls_external_only_var;

// Public variable (TREE_PUBLIC)
extern __thread int tls_public_var;

// Complex type to test different tree node configurations
struct ComplexStruct {
    int a;
    double b;
    void* c;
};

extern __thread struct ComplexStruct tls_struct_var __attribute__((used));

// Function declarations
void init_tls_vars(void);
size_t compute_tls_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_addresses(void);

#endif // TLS_VARS_H
