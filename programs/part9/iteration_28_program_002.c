/* tls_decl.h - Declarations for TLS variables with various attributes */

#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Visibility attribute for DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
#ifdef __GNUC__
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define DEFAULT_VIS __attribute__((visibility("default")))
#else
#define HIDDEN_VIS
#define DEFAULT_VIS
#endif

/* Weak attribute for DECL_WEAK */
#ifdef __GNUC__
#define WEAK_ATTR __attribute__((weak))
#else
#define WEAK_ATTR
#endif

/* Used attribute for DECL_PRESERVE_P */
#ifdef __GNUC__
#define USED_ATTR __attribute__((used))
#else
#define USED_ATTR
#endif

/* Common attribute for DECL_COMMON */
#ifdef __GNUC__
#define COMMON_ATTR __attribute__((common))
#else
#define COMMON_ATTR
#endif

/* Windows-specific attributes for DECL_DLLIMPORT_P */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* 
 * External declaration with multiple attributes:
 * - TREE_PUBLIC: yes (file-scope, external linkage)
 * - DECL_EXTERNAL: yes (declaration, not definition)
 * - DECL_WEAK: yes (weak attribute)
 * - DECL_VISIBILITY: hidden
 * - DECL_DLLIMPORT_P: on Windows
 */
extern DLL_IMPORT WEAK_ATTR HIDDEN_VIS int tls_var;

/* Another TLS variable with default visibility */
extern DEFAULT_VIS long tls_var2;

/* Function to use the TLS variables */
void use_tls_variables(void);

#ifdef __cplusplus
}
#endif

#endif /* TLS_DECL_H */
