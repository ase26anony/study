#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with various attributes in header
// These will be extern declarations that need cloning

// 1. Used attribute - sets TREE_USED
extern __thread int tls_used_var __attribute__((used));

// 2. Weak attribute - sets DECL_WEAK
extern __thread int tls_weak_var __attribute__((weak));

// 3. Hidden visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

// 4. Default visibility - also sets visibility attributes
extern __thread int tls_default_var __attribute__((visibility("default")));

// 5. DLL import simulation (for DECL_DLLIMPORT_P)
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
// Simulate with visibility and weak
extern __thread int tls_dllimport_var __attribute__((weak, visibility("default")));
#endif

// 6. External declaration only - sets DECL_EXTERNAL
extern __thread int tls_external_only;

// 7. Common linkage test - affects DECL_COMMON
extern __thread int tls_common_var;

// 8. Public variable - affects TREE_PUBLIC
extern __thread int tls_public_var;

// 9. Struct type for complex cases
struct tls_struct {
    int a;
    double b;
    void* c;
};
extern __thread struct tls_struct tls_complex_var;

// 10. Different type: double
extern __thread double tls_double_var;

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void* get_tls_address(int index, int iteration);

#endif // TLS_VARS_H
