/* tls.h - Header for TLS variable declarations */
#ifndef TLS_H
#define TLS_H

/* Visibility attributes for DECL_VISIBILITY testing */
#define DLL_PUBLIC __attribute__((visibility("default")))
#define DLL_HIDDEN __attribute__((visibility("hidden")))

/* Windows-style attributes for DECL_DLLIMPORT_P testing */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#endif

/* External TLS declaration with visibility - triggers DECL_VISIBILITY_SPECIFIED */
extern DLL_PUBLIC __thread int external_tls;

/* Weak external declaration - triggers DECL_WEAK */
extern __thread int weak_tls __attribute__((weak));

/* DLL imported TLS - triggers DECL_DLLIMPORT_P */
extern DLL_IMPORT __thread int imported_tls;

/* Common linkage test - triggers DECL_COMMON */
extern __thread int common_tls;

/* Function prototypes */
int use_tls_variables(void);
int* get_tls_addresses(void);

#endif /* TLS_H */
