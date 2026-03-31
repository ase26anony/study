#ifndef TLS_H
#define TLS_H

/* Visibility attributes for DECL_VISIBILITY testing */
#define HIDDEN __attribute__((visibility("hidden")))
#define DEFAULT_VIS __attribute__((visibility("default")))

/* Weak linkage for DECL_WEAK */
#define WEAK_SYM __attribute__((weak))

/* DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS declaration with visibility */
extern __thread int external_tls DEFAULT_VIS;

/* DLL imported TLS (simulated) */
extern DLL_IMPORT __thread int imported_tls;

/* Weak external TLS */
extern __thread int weak_external_tls WEAK_SYM;

/* Function prototypes */
int use_tls_variables(void);
int* get_tls_addresses(void);

/* Opaque function to prevent optimization */
void opaque_tls_use(int* addr) __attribute__((noipa));

#endif /* TLS_H */
