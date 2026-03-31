/* tls_common.h - Common TLS declarations with various attributes */

#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public TLS variable - sets TREE_PUBLIC */
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

/* Used attribute - influences TREE_USED */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol (via tentative definition) */
extern __thread int common_tls_var;

/* TLS variable with preserve attribute */
extern __thread int preserve_tls_var __attribute__((used, noinline));

/* Function declarations */
void init_tls_vars(void);
int check_tls_vars(void);
void* thread_func(void* arg);

#ifdef __cplusplus
}
#endif

#endif /* TLS_COMMON_H */
