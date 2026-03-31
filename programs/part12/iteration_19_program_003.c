#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External TLS declaration with visibility and DLL import attributes */
extern __thread int external_tls 
    __attribute__((visibility("default")))
    DLL_IMPORT;

/* Function prototypes */
int __attribute__((noinline)) use_tls_variables(void);
int __attribute__((noinline)) take_tls_addresses(void);
void __attribute__((noipa)) opaque_function(void*);

#endif /* TLS_H */
