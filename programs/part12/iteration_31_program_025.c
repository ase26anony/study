#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

/* TLS variables with various attributes to be cloned */

/* Used attribute sets TREE_USED */
extern __thread int tls_used_var __attribute__((used));

/* Weak attribute sets DECL_WEAK */
extern __thread int tls_weak_var __attribute__((weak));

/* Hidden visibility sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

/* Default visibility */
extern __thread int tls_default_var __attribute__((visibility("default")));

/* DLL import simulation */
extern __thread int tls_dllimport_var __attribute__((dllimport));

/* Common linkage variable */
extern __thread int tls_common_var;

/* External only (DECL_EXTERNAL set) */
extern __thread int tls_external_only_var;

/* Complex type with multiple attributes */
extern __thread struct {
    int a;
    double b;
} tls_struct_var __attribute__((used, visibility("hidden")));

/* Function prototypes */
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_address(int index);

#endif /* TLS_VARS_H */
