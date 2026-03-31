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

/* DLL import simulation (DECL_DLLIMPORT_P) */
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

/* Common linkage (DECL_COMMON) */
extern __thread int tls_common_var;

/* External declaration (DECL_EXTERNAL) */
extern __thread int tls_external_var;

/* Public variable (TREE_PUBLIC) */
extern __thread int tls_public_var;

/* Static TLS variable with thread_local */
extern thread_local int tls_static_var;

/* Complex type for varied tree node configurations */
struct tls_struct {
    int a;
    double b;
    void* c;
};

extern __thread struct tls_struct tls_complex_var;

/* Function prototypes */
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void* get_tls_address(int index, int loop_iter);

#endif /* TLS_VARS_H */
