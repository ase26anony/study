/* tls_decls.h - TLS variable declarations with various attributes */
#ifndef TLS_DECLS_H
#define TLS_DECLS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public TLS variable - will have TREE_PUBLIC set */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* TLS variables with different visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));
extern __thread int default_tls_var __attribute__((visibility("default")));

/* Used attribute - ensures TREE_USED is set */
extern __thread int used_tls_var __attribute__((used));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Common symbol - tentative definition */
extern __thread int common_tls_var;

/* External declaration (will be defined in another file) */
extern __thread int external_tls_var;

/* Preserve attribute (affects DECL_PRESERVE_P) */
extern __thread int preserve_tls_var __attribute__((used, noinline));

/* Function declarations */
void init_tls_vars(void);
int check_tls_vars(void);
void* thread_func(void* arg);

#ifdef __cplusplus
}
#endif

#endif /* TLS_DECLS_H */
