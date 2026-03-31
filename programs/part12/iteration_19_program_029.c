#ifndef TLS_H
#define TLS_H

/* Visibility attributes for DECL_VISIBILITY testing */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((visibility("default")))
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* External TLS declaration with visibility and DLL import attributes */
extern __thread int external_tls 
    __attribute__((visibility("hidden")))
    __attribute__((used));

/* Weak external TLS declaration */
extern __thread int weak_external_tls 
    __attribute__((weak))
    __attribute__((visibility("default")));

/* Common linkage test - tentative definition */
extern __thread int common_tls;

/* Function prototypes */
int get_checksum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noinline));

/* Opaque function to prevent optimization */
void opaque_use(void* ptr) __attribute__((noipa));

#endif /* TLS_H */
