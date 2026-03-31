#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_vis_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;  // Just extern, no definition in header

// Different types
extern __thread double tls_double_var __attribute__((used));
extern __thread struct Point {
    int x;
    int y;
} tls_struct_var __attribute__((visibility("hidden")));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_address(int var_id);

// Enum to identify TLS variables
enum TLSVarID {
    VAR_USED,
    VAR_WEAK,
    VAR_HIDDEN,
    VAR_DEFAULT_VIS,
    VAR_EXTERN,
    VAR_DOUBLE,
    VAR_STRUCT,
    VAR_COUNT
};

#endif // TLS_VARS_H
