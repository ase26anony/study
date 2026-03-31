#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with various attributes in header
// These will be extern declarations that get cloned in other compilation units

// 1. Used attribute - sets TREE_USED
extern __thread int tls_used_var __attribute__((used));

// 2. Weak attribute - sets DECL_WEAK
extern __thread int tls_weak_var __attribute__((weak));

// 3. Hidden visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

// 4. Default visibility - also sets visibility flags
extern __thread int tls_default_var __attribute__((visibility("default")));

// 5. DLL import style attribute (simulated) - sets DECL_DLLIMPORT_P
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
// On non-Windows, use a visibility attribute that might trigger similar paths
extern __thread int tls_dllimport_var __attribute__((visibility("protected")));
#endif

// 6. External declaration - sets DECL_EXTERNAL
extern __thread int tls_external_var;

// 7. Common linkage variable - sets DECL_COMMON when appropriate
extern __thread int tls_common_var;

// 8. Public variable - should set TREE_PUBLIC
extern __thread int tls_public_var;

// 9. Different types and storage
extern __thread double tls_double_var;
extern __thread struct {
    int a;
    double b;
} tls_struct_var;

// Function declarations
void init_tls_vars(void);
size_t compute_tls_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_addresses(int index);

#endif // TLS_VARS_H
