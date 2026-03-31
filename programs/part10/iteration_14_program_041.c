#ifndef TLS_DECLS_H
#define TLS_DECLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Extern TLS declaration with various attributes */
extern __thread int extern_tls_var __attribute__((used, weak, visibility("default")));

/* DLL imported TLS variable */
extern DLL_IMPORT __thread long imported_tls_var;

/* Function prototypes */
void init_tls_values(void);
unsigned long compute_checksum(void);
void opaque_operation(int* a, long* b, double* c);

#endif
