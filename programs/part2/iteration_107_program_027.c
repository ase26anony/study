/* tls_defs.h - TLS variable declarations with various attributes */

#ifndef TLS_DEFS_H
#define TLS_DEFS_H

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
#else
extern __thread int imported_tls_var;
#endif

/* Used attribute to ensure TREE_USED is set */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol (tentative definition) */
extern __thread int common_tls_var;

/* TLS variable with multiple attributes combined */
extern __thread int multi_attr_tls_var 
    __attribute__((weak, visibility("hidden"), used));

/* Function to access TLS variables */
void access_tls_variables(void);
int compute_tls_checksum(void);

#ifdef __cplusplus
}
#endif

#endif /* TLS_DEFS_H */
