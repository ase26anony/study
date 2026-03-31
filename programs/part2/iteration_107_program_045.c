/* tls_attributes.h - Header file with TLS variable declarations */

#ifndef TLS_ATTRIBUTES_H
#define TLS_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public TLS variable with external linkage */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* TLS variables with different visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));
extern __thread int internal_tls_var __attribute__((visibility("internal")));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Used attribute - forces TREE_USED to be set */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol - tentative definition */
extern __thread int common_tls_var;

/* Preserve attribute - influences DECL_PRESERVE_P */
extern __thread int preserve_tls_var __attribute__((used, noinline));

/* TLS variable with multiple attributes */
extern __thread int multi_attr_tls_var __attribute__((weak, visibility("hidden"), used));

/* Function prototypes */
void init_tls_variables(void);
int verify_tls_variables(void);
void* thread_function(void* arg);

#ifdef __cplusplus
}
#endif

#endif /* TLS_ATTRIBUTES_H */
