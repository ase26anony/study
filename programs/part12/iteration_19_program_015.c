#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS with visibility and DLL import attributes */
extern DLL_IMPORT __thread int external_tls 
    __attribute__((visibility("default"), weak));

/* Function prototypes */
int get_checksum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noinline));

#endif /* TLS_H */
