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

/* Global TLS with default visibility - triggers DECL_VISIBILITY */
extern __thread int tls_global_default;

/* Global TLS with hidden visibility - triggers DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_global_hidden 
    __attribute__((visibility("hidden")));

/* Weak TLS declaration - triggers DECL_WEAK */
extern __thread int tls_weak 
    __attribute__((weak));

/* DLL imported TLS - triggers DECL_DLLIMPORT_P */
extern DLL_IMPORT __thread int tls_dllimport;

/* Common TLS - triggers DECL_COMMON when combined with extern */
extern __thread int tls_common;

/* Function prototypes */
int get_tls_sum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noinline));
void preserve_tls_vars(void) __attribute__((used));

#endif /* TLS_H */
