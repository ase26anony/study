#ifndef TLS_DECL_H
#define TLS_DECL_H

#include <stddef.h>

/* Declare TLS variables with various attributes */
extern __thread int extern_tls_int __attribute__((used, weak, visibility("default")));

/* For Windows DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

extern DLL_IMPORT __thread long extern_tls_long;

/* Function prototypes */
void init_tls_values(void);
size_t compute_tls_checksum(void);
void opaque_operation(int* a, long* b, double* c);

#endif /* TLS_DECL_H */
