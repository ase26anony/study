#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

/* Thread-local variables with various attributes */

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

/* External declaration only (DECL_EXTERNAL) */
extern __thread int tls_external_only_var;

/* Public variable (TREE_PUBLIC) */
extern __thread int tls_public_var;

/* Struct type to test complex types */
struct tls_struct {
    int a;
    double b;
    void* c;
};

/* TLS struct with multiple attributes */
extern __thread struct tls_struct tls_complex_var 
    __attribute__((used, visibility("hidden")));

/* Function declarations */
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void* get_tls_address(int index);

#endif /* TLS_VARS_H */
