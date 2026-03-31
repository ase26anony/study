#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Declare TLS variables with various attributes */
extern __thread int extern_tls_var __attribute__((used));
extern __thread long extern_tls_weak_var __attribute__((weak, visibility("default")));
DLL_IMPORT extern __thread double imported_tls_var;

/* Function prototypes */
void init_tls_values(void);
unsigned long compute_tls_checksum(void);
void opaque_operation(void* ptr1, void* ptr2) __attribute__((noinline));

#endif /* TLS_DECL_H */
