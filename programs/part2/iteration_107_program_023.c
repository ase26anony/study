#ifndef TLS_ATTRS_H
#define TLS_ATTRS_H

#include <stdio.h>

/* Public TLS variable - sets TREE_PUBLIC */
extern __thread int public_tls_var;

/* Weak TLS variable */
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

/* Common symbol (tentative definition) */
extern __thread int common_tls_var;

/* Used attribute */
extern __thread int used_tls_var __attribute__((used));

/* External/Common test variable */
extern __thread int external_common_tls_var;

/* Function prototypes */
void init_tls_vars(void);
void print_tls_vars(void);
void* thread_func(void* arg);

#endif /* TLS_ATTRS_H */
