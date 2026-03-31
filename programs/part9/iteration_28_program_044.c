/* tls_decl.h - Declarations for TLS variables with various attributes */

#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Visibility attribute for DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* Weak attribute for DECL_WEAK */
#ifdef __GNUC__
#define WEAK_ATTR __attribute__((weak))
#else
#define WEAK_ATTR
#endif

/* Visibility attribute */
#ifdef __GNUC__
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#else
#define HIDDEN_VIS
#endif

/* External declaration with multiple attributes */
extern WEAK_ATTR HIDDEN_VIS DLL_IMPORT __thread int tls_var_external;

/* Common attribute for DECL_COMMON */
#ifdef __GNUC__
#define COMMON_ATTR __attribute__((common))
#else
#define COMMON_ATTR
#endif

/* Function to access TLS variables */
int get_tls_value(void);
void set_tls_value(int val);

#ifdef __cplusplus
}
#endif

#endif /* TLS_DECL_H */
