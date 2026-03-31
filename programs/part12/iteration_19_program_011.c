/* tls.h - TLS declarations with various attributes */
#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_ATTR __declspec(dllimport)
#else
#define DLL_ATTR
#endif

/* External TLS with visibility and DLL attributes */
extern __thread int external_tls 
    __attribute__((visibility("default")))
    DLL_ATTR;

/* Weak external TLS */
extern __thread int weak_external_tls 
    __attribute__((weak))
    __attribute__((visibility("hidden")));

/* Common TLS (tentative definition) */
extern __thread int common_tls;

/* Function prototypes */
int __attribute__((noinline)) use_tls_variables(void);
int __attribute__((noinline)) take_tls_addresses(void);
void __attribute__((noipa)) opaque_function(void*);

#endif /* TLS_H */
