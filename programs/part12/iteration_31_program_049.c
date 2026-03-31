#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

/* TLS variable declarations with various attributes */

/* Used attribute - sets TREE_USED */
extern __thread int tls_used_var __attribute__((used));

/* Weak attribute - sets DECL_WEAK */
extern __thread int tls_weak_var __attribute__((weak));

/* Hidden visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

/* Default visibility - also sets visibility attributes */
extern __thread int tls_default_var __attribute__((visibility("default")));

/* DLL import style (simulated with dllimport-like attribute) */
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

/* Common linkage variable */
extern __thread int tls_common_var;

/* External-only declaration (DECL_EXTERNAL will be true) */
extern __thread int tls_external_only_var;

/* Public variable (TREE_PUBLIC will be true) */
extern __thread int tls_public_var;

/* Variable with multiple attributes */
extern __thread int tls_multi_attr_var __attribute__((used, weak, visibility("hidden")));

/* Different types for variety */
extern __thread double tls_double_var __attribute__((used));
extern __thread struct {
    int a;
    double b;
    char c;
} tls_struct_var __attribute__((visibility("default")));

/* Function prototypes */
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_address(int var_index);

/* Enum to identify TLS variables */
enum TLS_VAR_INDEX {
    TLS_USED_VAR,
    TLS_WEAK_VAR,
    TLS_HIDDEN_VAR,
    TLS_DEFAULT_VAR,
    TLS_DLLIMPORT_VAR,
    TLS_COMMON_VAR,
    TLS_EXTERNAL_VAR,
    TLS_PUBLIC_VAR,
    TLS_MULTI_ATTR_VAR,
    TLS_DOUBLE_VAR,
    TLS_STRUCT_VAR,
    TLS_VAR_COUNT
};

#endif /* TLS_VARS_H */
