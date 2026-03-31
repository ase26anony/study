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

/* DLL import style (simulated with dllimport-like attribute) */
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

/* Common linkage variable */
extern __thread int tls_common_var;

/* External-only declaration (no definition in this TU) */
extern __thread int tls_external_var;

/* Static TLS in header (different linkage context) */
static __thread int tls_static_inline_var;

/* Different types for variety */
extern __thread double tls_double_var;
extern __thread char tls_char_var;

/* Struct type TLS variable */
struct TLSStruct {
    int a;
    double b;
    char c;
};
extern __thread struct TLSStruct tls_struct_var;

/* Function prototypes */
void init_tls_vars(void);
unsigned long compute_tls_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_addresses(int idx);

#endif /* TLS_VARS_H */
