#ifndef TLS_DEFS_H
#define TLS_DEFS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#endif

// Declare external TLS variables that will be defined in file2.c
extern __thread int tls_external_var;
extern __thread int tls_external_weak_var __attribute__((weak));
extern DLL_IMPORT __thread int tls_dllimport_var;

// Function prototypes
void use_tls_variables(void);
void modify_tls_variables(int val);
unsigned long compute_tls_checksum(void);

#endif // TLS_DEFS_H
