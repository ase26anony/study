#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS with visibility and DLL import attributes */
extern __thread int external_tls 
    __attribute__((visibility("default")))
    DLL_IMPORT;

/* Weak external TLS declaration */
extern __thread int weak_tls 
    __attribute__((weak))
    __attribute__((visibility("hidden")));

/* Function prototypes */
int __attribute__((noinline)) use_tls_variables(void);
void __attribute__((noinline)) take_tls_addresses(void);
extern void opaque_function(void*);  /* Opaque external function */

#endif /* TLS_H */
