/* tls_decls.h - TLS variable declarations with diverse attributes */

#ifndef TLS_DECLS_H
#define TLS_DECLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Public TLS variable with external linkage */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* TLS variables with different visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));
extern __thread int internal_tls_var __attribute__((visibility("internal")));

/* DLL import attribute (platform-specific) */
#ifdef TEST_DLLIMPORT
extern DLL_IMPORT __thread int imported_tls_var;
#endif

/* Common symbol (tentative definition) */
extern __thread int common_tls_var;

/* Used attribute to ensure not optimized away */
extern __thread int used_tls_var __attribute__((used));

/* Preserve attribute */
extern __thread int preserve_tls_var __attribute__((used, noinline));

/* Thread-local pointer */
extern __thread int* tls_ptr_var;

/* Function declarations */
void init_tls_vars(void);
void modify_tls_vars(void);
int check_tls_values(void);
void* thread_func(void* arg);

#endif /* TLS_DECLS_H */
