#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Extern TLS declaration with various attributes */
extern __thread int extern_tls_used __attribute__((used));
extern __thread long extern_tls_weak __attribute__((weak));
extern __thread double extern_tls_hidden __attribute__((visibility("hidden")));
extern DLL_IMPORT __thread int extern_tls_dllimport;

/* Function prototypes */
void init_tls_values(void);
unsigned long compute_checksum(void);
void opaque_operation(void* ptr1, void* ptr2);

#endif /* TLS_DECL_H */
