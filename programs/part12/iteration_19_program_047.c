#ifndef TLS_H
#define TLS_H

/* Visibility attributes */
#define TLS_VIS_HIDDEN __attribute__((visibility("hidden")))
#define TLS_VIS_DEFAULT __attribute__((visibility("default")))
#define TLS_WEAK __attribute__((weak))
#define TLS_USED __attribute__((used))
#define TLS_DLLIMPORT __attribute__((dllimport))

/* For Windows/MinGW compatibility */
#ifdef _WIN32
# define DLL_IMPORT __declspec(dllimport)
#else
# define DLL_IMPORT
#endif

/* External TLS declaration with visibility */
extern TLS_VIS_HIDDEN __thread int external_tls;

/* Weak external TLS */
extern TLS_WEAK __thread int weak_external_tls;

/* DLL import style declaration */
extern DLL_IMPORT __thread int dllimport_tls;

/* Common linkage test */
extern __thread int common_tls;

/* Function prototypes */
unsigned int use_tls_variables(void);
unsigned int take_tls_addresses(void);

#endif /* TLS_H */
