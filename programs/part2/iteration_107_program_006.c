/* tls_attrs.h - TLS variable declarations with various attributes */

#ifndef TLS_ATTRS_H
#define TLS_ATTRS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public TLS variable - sets TREE_PUBLIC */
extern __thread int public_tls_var;

/* Weak TLS variable - sets DECL_WEAK */
extern __thread int weak_tls_var __attribute__((weak));

/* Visibility attributes - sets DECL_VISIBILITY */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));

/* DLL Import attribute (Windows-specific) - sets DECL_DLLIMPORT_P */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#else
extern __thread int imported_tls_var;
#endif

/* Used attribute - influences TREE_USED */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol (tentative definition in one file, definition in another) */
extern __thread int common_tls_var;

/* External declaration (defined elsewhere) */
extern __thread int external_tls_var;

/* Additional test variables with combinations */
extern __thread int public_weak_tls_var __attribute__((weak));
extern __thread int public_hidden_tls_var __attribute__((visibility("hidden")));

/* Function prototypes */
void init_tls_vars(void);
int check_tls_vars(void);
void* thread_func(void* arg);

#ifdef __cplusplus
}
#endif

#endif /* TLS_ATTRS_H */
