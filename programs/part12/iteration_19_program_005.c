#ifndef TLS_H
#define TLS_H

// External TLS declaration with visibility attribute
extern __thread int external_tls __attribute__((visibility("default")));

// Weak external declaration
extern __thread int weak_tls __attribute__((weak));

// DLL import simulation (use appropriate attribute for platform)
#ifdef _WIN32
extern __thread int dllimport_tls __attribute__((dllimport));
#else
// On non-Windows, use a different mechanism or skip
extern __thread int dllimport_tls;
#endif

// Function prototypes
int get_checksum(void);
void use_tls_addresses(void);

#endif // TLS_H
