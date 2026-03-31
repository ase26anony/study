#ifndef TLS_ATTRIBUTES_H
#define TLS_ATTRIBUTES_H

/* Public TLS variable - will have TREE_PUBLIC set */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* Visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Used attribute - influences TREE_USED */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol - tentative definition */
extern __thread int common_tls_var;

/* External declaration for cross-file testing */
extern __thread int external_tls_var;

/* Function prototypes */
void init_tls_variables(void);
int compute_tls_checksum(void);
void* thread_function(void* arg);

#endif /* TLS_ATTRIBUTES_H */
