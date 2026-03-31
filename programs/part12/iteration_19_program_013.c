#ifndef TLS_H
#define TLS_H

// External TLS declaration with visibility and DLL attributes
#ifdef _WIN32
#define DLL_ATTR __declspec(dllimport)
#else
#define DLL_ATTR
#endif

// External TLS variable with visibility attribute
extern __thread int external_tls __attribute__((visibility("default")));

// Function prototypes
int get_checksum(void);
void use_tls_addresses(void);

#endif // TLS_H
