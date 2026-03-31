#ifndef TLS_H
#define TLS_H

/* Visibility attributes for DECL_VISIBILITY coverage */
#ifdef _WIN32
  #define DLL_IMPORT __declspec(dllimport)
  #define DLL_EXPORT __declspec(dllexport)
#else
  #define DLL_IMPORT __attribute__((dllimport))
  #define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* External TLS declaration with visibility and DLL import */
extern __thread int external_tls 
    __attribute__((visibility("hidden")))
    DLL_IMPORT;

/* Weak external TLS for DECL_WEAK coverage */
extern __thread int weak_tls 
    __attribute__((weak))
    __attribute__((visibility("default")));

/* Common linkage test - tentative definition */
extern __thread int common_tls;  /* DECL_COMMON will be set */

/* Function prototypes */
int __attribute__((noinline)) use_tls_variables(volatile int seed);
void __attribute__((noipa)) opaque_tls_use(int* addr);

#endif /* TLS_H */
