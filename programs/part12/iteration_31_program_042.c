#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with various attributes in header
// These will be extern declarations that get cloned in other compilation units

// Used attribute - sets TREE_USED
extern __thread int tls_used_var __attribute__((used));

// Weak attribute - sets DECL_WEAK
extern __thread int tls_weak_var __attribute__((weak));

// Hidden visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

// Default visibility - also sets visibility attributes
extern __thread int tls_default_var __attribute__((visibility("default")));

// DLL import simulation (for DECL_DLLIMPORT_P)
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
// Simulate with a different attribute
extern __thread int tls_dllimport_var __attribute__((weak));
#endif

// External declaration only (DECL_EXTERNAL = 1)
extern __thread int tls_external_only_var;

// Common linkage variable (DECL_COMMON)
extern __thread int tls_common_var;

// Public variable (TREE_PUBLIC)
extern __thread int tls_public_var;

// Static TLS in header (different linkage context)
static __thread int tls_static_in_header;

// Complex type with struct
struct ComplexTLS {
    int a;
    double b;
    void* c;
};

extern __thread struct ComplexTLS tls_struct_var;

// Function declarations
void init_tls_vars(int seed);
unsigned long compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_address(int index);

// Enum to identify TLS variables
enum TLSVarIndex {
    TLS_USED = 0,
    TLS_WEAK,
    TLS_HIDDEN,
    TLS_DEFAULT,
    TLS_DLLIMPORT,
    TLS_EXTERNAL_ONLY,
    TLS_COMMON,
    TLS_PUBLIC,
    TLS_STRUCT,
    TLS_COUNT
};

#endif // TLS_VARS_H
