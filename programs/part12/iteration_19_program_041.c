#ifndef TLS_H
#define TLS_H

/* Visibility attributes for DECL_VISIBILITY testing */
#ifdef _WIN32
  #define DLL_IMPORT __declspec(dllimport)
  #define DLL_EXPORT __declspec(dllexport)
#else
  #define DLL_IMPORT __attribute__((dllimport))
  #define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* Global TLS with default visibility */
extern __thread int tls_global_default;

/* Global TLS with hidden visibility */
extern __thread int tls_global_hidden __attribute__((visibility("hidden")));

/* Weak TLS declaration */
extern __thread int tls_weak __attribute__((weak));

/* DLL imported TLS (Windows-specific or simulated) */
extern DLL_IMPORT __thread int tls_imported;

/* Common linkage TLS (tentative definition) */
extern __thread int tls_common;

/* Function prototypes */
int compute_checksum(void);
void* get_tls_addresses(void);

#endif /* TLS_H */
