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

/* Default visibility - also sets visibility attributes */
extern __thread int tls_default_var __attribute__((visibility("default")));

/* DLL import simulation (DECL_DLLIMPORT_P) - using dllimport attribute */
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
/* On non-Windows, use a different mechanism to potentially set DECL_DLLIMPORT_P */
extern __thread int tls_dllimport_var __attribute__((weak));
#endif

/* External declaration only - sets DECL_EXTERNAL */
extern __thread int tls_external_only;

/* Common linkage simulation - may affect DECL_COMMON */
extern __thread int tls_common_var;

/* Public variable - affects TREE_PUBLIC */
extern __thread int tls_public_var;

/* Static TLS variable in header - different linkage context */
static __thread int tls_static_in_header;

/* Complex type with TLS */
struct ComplexTLS {
    int a;
    double b;
    void* c;
};

extern __thread struct ComplexTLS tls_struct_var __attribute__((used));

/* Function declarations */
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_addresses(void);

#endif /* TLS_VARS_H */
