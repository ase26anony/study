#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External TLS declaration with visibility attribute */
extern __thread int external_tls __attribute__((visibility("default")));

/* DLL imported TLS (for Windows targets) */
extern DLL_IMPORT __thread int imported_tls;

/* Weak external TLS */
extern __thread int weak_tls __attribute__((weak));

/* Function prototypes */
int get_checksum(void);
void use_tls_addresses(void);

#endif /* TLS_H */
