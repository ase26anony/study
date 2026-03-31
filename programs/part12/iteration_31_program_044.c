#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with diverse attributes in header
// These will be extern declarations that need cloning in other compilation units

// 1. Weak TLS variable with used attribute
extern __thread int tls_weak_used __attribute__((weak, used));

// 2. TLS with hidden visibility
extern __thread double tls_hidden __attribute__((visibility("hidden")));

// 3. TLS with default visibility (explicitly specified)
extern __thread long tls_default_vis __attribute__((visibility("default")));

// 4. Common TLS variable (tentative definition)
extern __thread volatile int tls_common;

// 5. TLS variable that might be dllimport (simulated via weak)
extern __thread char tls_dllimport __attribute__((weak));

// 6. Static TLS in header (internal linkage)
static __thread int tls_static_internal = 42;

// 7. TLS with multiple attributes
extern __thread struct Point {
    int x;
    int y;
} tls_struct __attribute__((used, visibility("hidden")));

// 8. External TLS without definition in this TU
extern __thread size_t tls_external_only;

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_address(int index);

// Enum to identify TLS variables
enum TLSVarIndex {
    TLS_WEAK_USED,
    TLS_HIDDEN,
    TLS_DEFAULT_VIS,
    TLS_COMMON,
    TLS_DLLIMPORT,
    TLS_STRUCT,
    TLS_EXTERNAL_ONLY,
    TLS_COUNT
};

#endif // TLS_VARS_H
