#ifndef TLS_H
#define TLS_H

/* External TLS declaration with visibility attribute */
extern __thread int external_tls __attribute__((visibility("default")));

/* Weak TLS declaration */
extern __thread int weak_tls __attribute__((weak));

/* DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

extern DLL_IMPORT __thread int dllimport_tls;

/* Function prototypes */
int get_checksum(void);
void use_tls_addresses(void);

#endif /* TLS_H */
