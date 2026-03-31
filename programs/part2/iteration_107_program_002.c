/* tls_attributes.h - TLS variable declarations with various attributes */

#ifndef TLS_ATTRIBUTES_H
#define TLS_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public/External linkage - sets TREE_PUBLIC and potentially DECL_EXTERNAL */
extern __thread int public_tls_var;

/* Weak symbol attribute - sets DECL_WEAK */
extern __thread int weak_tls_var __attribute__((weak));

/* Visibility attributes - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));

/* DLL Import Attribute (Windows-specific) - sets DECL_DLLIMPORT_P */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#else
extern __thread int imported_tls_var;
#endif

/* Used attribute - influences TREE_USED */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol - will be set via tentative definition */
extern __thread int common_tls_var;

/* Additional test variables */
extern __thread int preserved_tls_var;  /* Should set DECL_PRESERVE_P */
extern __thread int context_tls_var;    /* Should set DECL_CONTEXT */

/* Function prototypes */
void init_tls_variables(void);
int compute_tls_checksum(void);
void* thread_function(void* arg);

#ifdef __cplusplus
}
#endif

#endif /* TLS_ATTRIBUTES_H */
