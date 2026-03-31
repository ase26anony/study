/* tls_attributes.h - TLS variable declarations with diverse attributes */

#ifndef TLS_ATTRIBUTES_H
#define TLS_ATTRIBUTES_H

/* Public/External linkage */
extern __thread int public_tls_var;

/* Weak symbol */
extern __thread int weak_tls_var __attribute__((weak));

/* Visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#else
extern __thread int imported_tls_var;
#endif

/* Common symbol (via tentative definition) */
extern __thread int common_tls_var;

/* Used attribute */
extern __thread int used_tls_var __attribute__((used));

/* Combination: weak + hidden */
extern __thread int weak_hidden_tls_var __attribute__((weak, visibility("hidden")));

/* Static TLS (non-public) for contrast */
static __thread int static_tls_var;

/* Function prototypes */
void init_tls_vars(void);
void modify_tls_vars(void);
int compute_tls_checksum(void);

#endif /* TLS_ATTRIBUTES_H */
