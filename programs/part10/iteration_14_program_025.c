#ifndef TLS_DECL_H
#define TLS_DECL_H

#include <stddef.h>

/* Declare TLS variables with various attributes */
extern __thread int extern_tls_var __attribute__((used, weak, visibility("default")));

/* For Windows DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Function prototypes */
size_t compute_checksum(void);
void init_tls_values(void);
void opaque_operation(int* a, int* b, int* c);

#endif /* TLS_DECL_H */
