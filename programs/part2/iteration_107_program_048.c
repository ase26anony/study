/* tls_attributes.h - TLS variable declarations with various attributes */

#ifndef TLS_ATTRIBUTES_H
#define TLS_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public TLS variable - sets TREE_PUBLIC */
extern __thread int public_tls_var;

/* Weak TLS variable - sets DECL_WEAK */
extern __thread int weak_tls_var __attribute__((weak));

/* Hidden visibility TLS variable - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));

/* Protected visibility TLS variable */
extern __thread int protected_tls_var __attribute__((visibility("protected")));

/* Used attribute TLS variable - influences TREE_USED */
extern __thread int used_tls_var __attribute__((used));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Common symbol - tentative definition */
extern __thread int common_tls_var;

/* External declaration only */
extern __thread int external_tls_var;

/* Function prototypes */
void init_tls_variables(void);
int check_tls_variables(void);
void modify_tls_from_thread(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* TLS_ATTRIBUTES_H */
