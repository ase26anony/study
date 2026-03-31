#ifndef TLS_DEFS_H
#define TLS_DEFS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

// Declare external TLS variables that will be defined in other files
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern DLL_IMPORT __thread int tls_dllimport;

#endif
