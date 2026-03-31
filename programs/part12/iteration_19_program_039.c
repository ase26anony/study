#ifndef TLS_H
#define TLS_H

/* Visibility attributes */
#ifdef _WIN32
  #define DLL_IMPORT __declspec(dllimport)
  #define DLL_EXPORT __declspec(dllexport)
#else
  #define DLL_IMPORT __attribute__((dllimport))
  #define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* Force emulated TLS */
#pragma GCC tls_model emulated

/* External TLS with various attributes */
extern __thread int tls_external_default 
    __attribute__((visibility("default")));

extern __thread int tls_external_hidden 
    __attribute__((visibility("hidden")))
    __attribute__((weak));

extern DLL_IMPORT __thread int tls_dllimported;

/* Common linkage simulation */
extern __thread int tls_common;

/* Used attribute to trigger DECL_PRESERVE_P */
extern __thread int tls_preserved 
    __attribute__((used))
    __attribute__((visibility("protected")));

/* Function prototypes */
int __attribute__((noinline)) use_tls_variables(int seed);
void __attribute__((noipa)) opaque_tls_access(int* addr);

#endif /* TLS_H */
