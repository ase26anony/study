#ifndef TLS_ATTRS_H
#define TLS_ATTRS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Public TLS variable - will be TREE_PUBLIC */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* Visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));
extern __thread int default_vis_tls_var __attribute__((visibility("default")));

/* DLL import attribute (platform-specific) */
#ifdef TEST_DLLIMPORT
extern __thread int imported_tls_var DLL_IMPORT;
#endif

/* Used attribute */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol (via tentative definition) */
extern __thread int common_tls_var;

/* External declaration (defined elsewhere) */
extern __thread int external_tls_var;

/* Static TLS (should not be public) */
__thread int static_tls_var;

/* Function prototypes */
void init_tls_vars(void);
int compute_checksum(void);
void* thread_func(void* arg);

#endif /* TLS_ATTRS_H */
