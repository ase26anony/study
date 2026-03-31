#ifndef TLS_H
#define TLS_H

/* Visibility attributes */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#endif

/* Force emulated TLS even if native is available */
#pragma GCC tls_model emulated

/* External TLS declaration with visibility and DLL import */
extern __thread int external_tls 
    __attribute__((visibility("default")))
    DLL_IMPORT;

/* Weak external TLS */
extern __thread int weak_external_tls 
    __attribute__((weak))
    __attribute__((visibility("hidden")));

/* Common TLS (tentative definition trigger) */
extern __thread int common_tls;

/* Function prototypes */
int get_checksum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noipa));

#endif /* TLS_H */
