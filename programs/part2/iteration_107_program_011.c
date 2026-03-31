/* tls_defs.h - TLS variable declarations with various attributes */
#ifndef TLS_DEFS_H
#define TLS_DEFS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#endif

/* Public TLS variable with external linkage */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* TLS variables with different visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));
extern __thread int default_tls_var __attribute__((visibility("default")));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread DLL_IMPORT int imported_tls_var;
#else
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Used attribute to ensure TREE_USED is set */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol (via tentative definition) */
extern __thread int common_tls_var;

/* TLS variable with preserve attribute */
extern __thread int preserve_tls_var __attribute__((used, noinline));

/* Function declarations */
void init_tls_vars(void);
void modify_tls_vars(void);
int check_tls_values(void);
void* thread_func(void* arg);

#endif /* TLS_DEFS_H */
