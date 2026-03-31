#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

/* Declare TLS variables with various attributes */

/* Used attribute - sets TREE_USED */
extern __thread int tls_used_var __attribute__((used));

/* Weak attribute - sets DECL_WEAK */
extern __thread int tls_weak_var __attribute__((weak));

/* Hidden visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

/* Default visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_default_var __attribute__((visibility("default")));

/* DLL import style (simulated with weak) - for DECL_DLLIMPORT_P */
extern __thread int tls_dllimport_var __attribute__((weak));

/* External only declaration - sets DECL_EXTERNAL */
extern __thread int tls_external_var;

/* Common linkage variable - for DECL_COMMON */
extern __thread int tls_common_var;

/* Public variable - for TREE_PUBLIC */
extern __thread int tls_public_var;

/* Complex type with struct */
struct tls_struct {
    int a;
    double b;
    void* c;
};

/* TLS struct with attributes */
extern __thread struct tls_struct tls_struct_var __attribute__((used, visibility("hidden")));

/* Function declarations */
void init_tls_vars(void);
unsigned long compute_tls_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_addresses(int idx);

#endif /* TLS_VARS_H */
