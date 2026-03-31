#ifndef TLS_H
#define TLS_H

/* Visibility attributes for DECL_VISIBILITY testing */
#define TLS_VISIBLE __attribute__((visibility("default")))
#define TLS_HIDDEN __attribute__((visibility("hidden")))

/* Weak linkage for DECL_WEAK testing */
#define TLS_WEAK __attribute__((weak))

/* DLL import simulation */
#ifdef _WIN32
#define TLS_IMPORT __attribute__((dllimport))
#define TLS_EXPORT __attribute__((dllexport))
#else
#define TLS_IMPORT
#define TLS_EXPORT
#endif

/* External TLS declaration with visibility and DLL import */
extern TLS_IMPORT TLS_VISIBLE __thread int external_tls;

/* Weak external TLS */
extern TLS_WEAK __thread int weak_external_tls;

/* Common TLS variable (tentative definition) */
extern __thread int common_tls;

/* Function prototypes */
int get_tls_checksum(void);
void* get_tls_addresses(void);

#endif /* TLS_H */
