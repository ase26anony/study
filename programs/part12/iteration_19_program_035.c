#ifndef TLS_H
#define TLS_H

#ifdef _WIN32
#define DLL_ATTR __declspec(dllimport)
#else
#define DLL_ATTR
#endif

/* External TLS with visibility and weak attributes */
extern __thread int external_tls 
    __attribute__((weak))
    __attribute__((visibility("default")));

/* DLL imported TLS for Windows targets */
extern DLL_ATTR __thread int imported_tls;

/* Common TLS variable (tentative definition) */
extern __thread int common_tls;

/* Function prototypes */
int use_tls_variables(void);
int* get_tls_addresses(void);

#endif /* TLS_H */
