#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

/* Declare TLS variables with various attributes */

/* Weak TLS variable */
extern __thread int tls_weak_var __attribute__((weak));

/* Used TLS variable */
extern __thread int tls_used_var __attribute__((used));

/* Hidden visibility TLS variable */
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

/* Default visibility TLS variable */
extern __thread int tls_default_var __attribute__((visibility("default")));

/* DLL import style (simulated via weak) */
extern __thread int tls_dllimport_var __attribute__((weak));

/* Common linkage TLS variable */
extern __thread int tls_common_var;

/* External TLS variable (declaration only) */
extern __thread int tls_external_var;

/* Public TLS variable */
extern __thread int tls_public_var;

/* Static TLS variable (different linkage context) */
static __thread int tls_static_var;

/* TLS variable with multiple attributes */
extern __thread int tls_multi_attr_var __attribute__((used, weak, visibility("hidden")));

/* Different types of TLS variables */
extern __thread double tls_double_var;
extern __thread char tls_char_var;
extern __thread size_t tls_size_var;

/* Struct TLS variable */
struct TLSStruct {
    int a;
    double b;
    char c;
};
extern __thread struct TLSStruct tls_struct_var;

/* Function declarations */
void init_tls_vars(void);
unsigned long compute_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_addresses(int idx);

#endif /* TLS_VARS_H */
