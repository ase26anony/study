/* tls.h - Header for TLS variable declarations */
#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External TLS with visibility attribute */
extern __thread int external_tls __attribute__((visibility("default")));

/* Weak external TLS */
extern __thread int weak_tls __attribute__((weak));

/* DLL imported TLS (for Windows targets) */
DLL_IMPORT extern __thread int imported_tls;

/* Function prototypes */
int get_checksum(void);
void use_tls_addresses(void);

#endif /* TLS_H */
