#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* Extern TLS declaration - will trigger DECL_EXTERNAL copying */
extern __thread int tls_extern_var;

/* Weak TLS declaration - will trigger DECL_WEAK copying */
extern __thread int tls_weak_var __attribute__((weak));

/* DLL import style declaration - targets DECL_DLLIMPORT_P */
extern DLL_IMPORT __thread int tls_imported_var;

/* Visibility specified declaration */
extern __thread int tls_visible_var __attribute__((visibility("default")));

/* Common attribute declaration */
extern __thread int tls_common_var __attribute__((common));

/* Function prototype */
int process_tls_values(int iteration);

#endif /* TLS_COMMON_H */
