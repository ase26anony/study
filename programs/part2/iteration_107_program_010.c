/* tls_attrs.h - TLS variable declarations with various attributes */
#ifndef TLS_ATTRS_H
#define TLS_ATTRS_H

#include <stdio.h>

/* Public TLS variable with external linkage */
extern __thread int public_tls_var;

/* Weak TLS variable */
extern __thread int weak_tls_var __attribute__((weak));

/* TLS variables with different visibility attributes */
extern __thread int hidden_tls_var __attribute__((visibility("hidden")));
extern __thread int protected_tls_var __attribute__((visibility("protected")));
extern __thread int internal_tls_var __attribute__((visibility("internal")));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#else
extern __thread int imported_tls_var;
#endif

/* Used attribute to ensure TREE_USED is set */
extern __thread int used_tls_var __attribute__((used));

/* Common symbol - tentative definition */
extern __thread int common_tls_var;

/* External declaration that will be defined elsewhere */
extern __thread int external_tls_var;

/* Function prototypes */
void init_tls_vars(void);
void modify_tls_vars(void);
int check_tls_values(void);
void* thread_func(void* arg);

#endif /* TLS_ATTRS_H */
