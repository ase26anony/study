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

/* Default visibility (explicit) */
extern __thread int tls_default_var __attribute__((visibility("default")));

/* DLL import style (sets DECL_DLLIMPORT_P) */
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

/* Common linkage variable (sets DECL_COMMON) */
extern __thread int tls_common_var;

/* External declaration only (sets DECL_EXTERNAL) */
extern __thread int tls_external_only_var;

/* Public variable (sets TREE_PUBLIC) */
extern __thread int tls_public_var;

/* Complex type to test different tree node configurations */
struct tls_struct {
    int a;
    double b;
    void* c;
};

/* TLS struct with multiple attributes */
extern __thread struct tls_struct tls_struct_var 
    __attribute__((used, visibility("hidden")));

/* Function declarations */
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_address(int var_index);

#endif /* TLS_VARS_H */
