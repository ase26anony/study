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

/* Weak TLS declaration */
extern __thread int weak_tls 
    __attribute__((weak, visibility("hidden")));

/* Function prototypes */
int get_checksum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noinline));

/* Opaque function to prevent optimization */
void opaque_function(void*) __attribute__((noipa));

#endif /* TLS_H */
