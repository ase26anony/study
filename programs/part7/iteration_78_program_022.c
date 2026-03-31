#ifndef TLS_DEFS_H
#define TLS_DEFS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Public TLS with explicit visibility */
extern __thread int tls_public_default __attribute__((visibility("default")));
extern __thread int tls_public_hidden __attribute__((visibility("hidden")));

/* Weak TLS variables */
extern __thread int tls_weak_var __attribute__((weak));

/* DLL imported TLS (simulated) */
extern DLL_IMPORT __thread int tls_dll_imported;

/* Common linkage TLS (tentative definition) */
extern __thread int tls_common;

/* Used attribute to force preservation */
extern __thread int tls_used_var __attribute__((used));

/* Function declarations */
void use_tls_variables(void);
void modify_tls_from_file2(void);
unsigned int compute_tls_checksum(void);

#endif /* TLS_DEFS_H */
