#ifndef TLS_COMMON_H
#define TLS_COMMON_H

/* Thread-local storage declarations with various attributes */

/* Extern declaration with dllimport attribute for DECL_DLLIMPORT_P */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((visibility("default")))
#endif

/* Extern TLS variable - will be defined in tls.c */
extern DLL_IMPORT __thread int tls_extern_var;

/* Weak TLS declaration - for DECL_WEAK */
extern __thread int tls_weak_var __attribute__((weak));

/* Function prototype */
int process_tls_values(int iteration);

/* Macro to conditionally change linkage */
#ifdef USE_STATIC_LINKAGE
#define TLS_LINKAGE static
#else
#define TLS_LINKAGE extern
#endif

/* Conditionally visible TLS */
TLS_LINKAGE __thread int tls_conditional_var 
    __attribute__((visibility("hidden")));

#endif /* TLS_COMMON_H */
