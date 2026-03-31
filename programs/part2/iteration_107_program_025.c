#ifndef TLS_PUBLIC_H
#define TLS_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public TLS variable - will be TREE_PUBLIC and DECL_EXTERNAL */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* Visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));

/* Used attribute */
extern __thread int used_tls_var __attribute__((used));

/* DLL import attribute (Windows specific) */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Common symbol - tentative definition */
extern __thread int common_tls_var;

/* Function to access all TLS variables */
void access_all_tls_vars(void);
int compute_tls_checksum(void);

#ifdef __cplusplus
}
#endif

#endif /* TLS_PUBLIC_H */
