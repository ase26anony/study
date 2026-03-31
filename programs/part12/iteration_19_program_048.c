#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External TLS declaration with visibility and DLL import attributes */
extern __thread int external_tls 
    __attribute__((visibility("default")))
    DLL_IMPORT;

/* Function to take address of TLS variables */
int* get_external_tls_addr(void);

/* Opaque function to prevent optimization */
void use_tls_ptr(void* ptr) __attribute__((noipa));

#endif /* TLS_H */
