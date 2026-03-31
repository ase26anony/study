#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with diverse attributes
// These will be cloned when used in other compilation units

// 1. Used attribute (sets TREE_USED)
extern __thread int tls_used_int __attribute__((used));

// 2. Weak attribute (sets DECL_WEAK)
extern __thread int tls_weak_int __attribute__((weak));

// 3. Hidden visibility (sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED)
extern __thread double tls_hidden_double __attribute__((visibility("hidden")));

// 4. Default visibility (sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED)
extern __thread double tls_default_double __attribute__((visibility("default")));

// 5. DLL import style (simulates DECL_DLLIMPORT_P)
extern __thread int tls_dllimport_int __attribute__((dllimport));

// 6. Common linkage variable (affects DECL_COMMON)
extern __thread long tls_common_long;

// 7. Public variable (affects TREE_PUBLIC)
extern __thread int tls_public_int;

// 8. External-only declaration (DECL_EXTERNAL remains true)
extern __thread int tls_external_only_int;

// 9. Struct type for variety
struct TLSData {
    int a;
    double b;
    char c;
};

extern __thread struct TLSData tls_struct __attribute__((used));

// 10. Static TLS in header (different linkage context)
static __thread int tls_static_in_header = 42;

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_via_pointers(int iteration);

// Helper to prevent optimization
extern void use_pointer(void* ptr) __attribute__((noinline));

#endif // TLS_VARS_H
