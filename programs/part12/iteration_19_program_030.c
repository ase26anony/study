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

/* External TLS with visibility and DLL import attributes */
extern DLL_IMPORT __thread int external_tls;

/* Weak external reference */
extern __thread int weak_tls __attribute__((weak));

/* Common linkage test */
extern __thread int common_tls;  /* Tentative definition */

/* Function prototypes */
int get_checksum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noinline));

#endif /* TLS_H */
