/* tls_attributes.h - TLS variable declarations with diverse attributes */

#ifndef TLS_ATTRIBUTES_H
#define TLS_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public/External linkage - sets TREE_PUBLIC and potentially DECL_EXTERNAL */
extern __thread int public_tls_var;

/* Weak symbol attribute - sets DECL_WEAK */
extern __thread int weak_tls_var __attribute__((weak));

/* Visibility attributes - set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));
extern __thread int default_tls_var __attribute__((visibility("default")));

/* DLL Import Attribute (Windows-specific) - sets DECL_DLLIMPORT_P */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Used attribute - influences TREE_USED */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol - sets DECL_COMMON when tentative definition */
extern __thread int common_tls_var;

/* Variable with preserve attribute - sets DECL_PRESERVE_P */
extern __thread int preserve_tls_var __attribute__((used, noinline));

/* Variable with context (for DECL_CONTEXT) - declared inside a struct */
struct container {
    __thread int nested_tls_var;
};

/* Function to access all TLS variables */
void access_all_tls_vars(void);
int compute_tls_checksum(void);

#ifdef __cplusplus
}
#endif

#endif /* TLS_ATTRIBUTES_H */
