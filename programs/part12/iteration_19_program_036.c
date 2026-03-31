#ifndef TLS_H
#define TLS_H

// External TLS declaration with visibility attribute
extern __thread int external_tls __attribute__((visibility("default")));

// Weak TLS declaration
extern __thread int weak_tls __attribute__((weak));

// DLL import simulation (for Windows targets)
#ifdef _WIN32
extern __thread int dllimport_tls __attribute__((dllimport));
#else
// Simulate with visibility on non-Windows
extern __thread int dllimport_tls __attribute__((visibility("hidden")));
#endif

// Function prototypes
unsigned int use_tls_variables(void);
unsigned int use_static_tls(void);
void* get_tls_addresses(void);

#endif // TLS_H
