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

/* Global TLS with default visibility - will set DECL_VISIBILITY */
extern __thread int tls_global_default;

/* Hidden visibility TLS - tests DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_global_hidden __attribute__((visibility("hidden")));

/* Weak TLS variable - tests DECL_WEAK */
extern __thread int tls_weak __attribute__((weak));

/* DLL imported TLS (Windows) - tests DECL_DLLIMPORT_P */
extern DLL_IMPORT __thread int tls_imported;

/* Common TLS variable - tests DECL_COMMON */
extern __thread int tls_common;

/* External TLS declaration - tests DECL_EXTERNAL */
extern __thread int tls_external;

/* Function prototypes */
int get_tls_sum(void);
void modify_tls_values(int seed);
void* get_tls_addresses(void);

#endif /* TLS_H */
