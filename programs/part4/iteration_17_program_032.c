#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* Extern TLS declaration - will trigger property copying */
extern DLL_IMPORT __thread int tls_extern_var;

/* Function prototype */
int process_tls_values(int x);

/* Conditional attribute macro */
#ifdef USE_WEAK_TLS
#define WEAK_ATTR __attribute__((weak))
#else
#define WEAK_ATTR
#endif

#ifdef USE_HIDDEN_VIS
#define VIS_ATTR __attribute__((visibility("hidden")))
#else
#define VIS_ATTR __attribute__((visibility("default")))
#endif

#endif /* TLS_COMMON_H */
