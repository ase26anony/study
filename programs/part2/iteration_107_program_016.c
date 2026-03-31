/* tls_decls.h - TLS variable declarations with various attributes */

#ifndef TLS_DECLS_H
#define TLS_DECLS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public TLS variable with external linkage */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* TLS variables with different visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));
extern __thread int internal_tls_var __attribute__((visibility("internal")));

/* Used attribute - forces compiler to keep the variable */
extern __thread int used_tls_var __attribute__((used));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Common symbol (tentative definition in C) */
extern __thread int common_tls_var;

/* TLS variable with multiple attributes */
extern __thread int multi_attr_tls_var __attribute__((weak, visibility("hidden"), used));

/* Function prototypes */
void init_tls_vars(void);
void modify_tls_vars(void);
int check_tls_values(void);
void print_tls_addresses(void);

#ifdef __cplusplus
}
#endif

#endif /* TLS_DECLS_H */
