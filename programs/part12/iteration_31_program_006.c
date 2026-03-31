#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with various attributes
// These will be extern declarations that get cloned in other compilation units

// 1. Used attribute (sets TREE_USED)
extern __thread int tls_used_var __attribute__((used));

// 2. Weak attribute (sets DECL_WEAK)
extern __thread int tls_weak_var __attribute__((weak));

// 3. Hidden visibility (sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED)
extern __thread double tls_hidden_var __attribute__((visibility("hidden")));

// 4. Default visibility (sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED)
extern __thread double tls_default_var __attribute__((visibility("default")));

// 5. DLL import style (simulates DECL_DLLIMPORT_P)
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

// 6. Common linkage variable (sets DECL_COMMON)
extern __thread long tls_common_var;

// 7. External only (sets DECL_EXTERNAL)
extern __thread char tls_external_var;

// 8. Public variable (sets TREE_PUBLIC)
extern __thread float tls_public_var;

// 9. Static TLS in header (different linkage context)
static __thread int tls_static_in_header;

// 10. Struct type TLS variable
struct Point {
    int x;
    int y;
    double z;
};
extern __thread struct Point tls_struct_var;

// Function declarations
void init_tls_vars(int seed);
unsigned long compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_addresses(void);

#endif // TLS_VARS_H
