#ifndef TLS_H
#define TLS_H

/* Visibility attributes for DECL_VISIBILITY testing */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((visibility("default")))
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* External TLS declaration with visibility - tests DECL_VISIBILITY* */
extern __thread int external_tls 
    __attribute__((visibility("hidden")));

/* Weak external TLS - tests DECL_WEAK */
extern __thread int weak_tls 
    __attribute__((weak, visibility("default")));

/* DLL import simulation for DECL_DLLIMPORT_P */
#ifdef _WIN32
extern __thread int dllimport_tls 
    __declspec(dllimport);
#else
extern __thread int dllimport_tls 
    __attribute__((visibility("default")));
#endif

/* Function prototypes */
int use_tls_variables(void);
int* get_tls_addresses(void);

#endif /* TLS_H */
