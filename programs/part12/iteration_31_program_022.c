#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with various attributes that will be copied during cloning

// 1. Used attribute (sets TREE_USED)
extern __thread int tls_used_var __attribute__((used));

// 2. Weak attribute (sets DECL_WEAK)
extern __thread int tls_weak_var __attribute__((weak));

// 3. Hidden visibility (sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED)
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

// 4. Default visibility (sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED)
extern __thread int tls_default_var __attribute__((visibility("default")));

// 5. DLL import style (simulating DECL_DLLIMPORT_P)
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

// 6. External declaration (sets DECL_EXTERNAL)
extern __thread int tls_external_var;

// 7. Common linkage (sets DECL_COMMON)
extern __thread int tls_common_var;

// 8. Public variable (sets TREE_PUBLIC)
extern __thread int tls_public_var;

// 9. Static TLS variable (different linkage context)
static __thread int tls_static_var;

// 10. Different types to test various tree node configurations
extern __thread double tls_double_var;
extern __thread struct Point {
    int x;
    int y;
} tls_struct_var;

// Function declarations
void init_tls_vars(void);
size_t compute_tls_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_addresses(int index);

#endif // TLS_VARS_H
