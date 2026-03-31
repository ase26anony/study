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
extern __thread int common_tls;

/* Function prototypes */
int __attribute__((noinline)) use_tls_variables(void);
int __attribute__((noinline)) take_tls_addresses(void);
void __attribute__((noipa)) opaque_function(void*);

#endif /* TLS_H */
