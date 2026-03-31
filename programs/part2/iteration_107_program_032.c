/* File: tls_public.h - Common declarations */
#ifndef TLS_PUBLIC_H
#define TLS_PUBLIC_H

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

/* DLL import attribute (Windows-specific) */
extern DLL_IMPORT __thread int imported_tls_var;

/* Used attribute */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol - tentative definition in one file, definition in another */
extern __thread int common_tls_var;

/* External declaration only */
extern __thread int external_only_tls_var;

/* Function prototypes */
void init_tls_vars(void);
void modify_tls_vars(void);
int compute_tls_checksum(void);

#endif /* TLS_PUBLIC_H */
