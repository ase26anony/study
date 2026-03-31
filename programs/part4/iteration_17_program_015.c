#ifndef TLS_H
#define TLS_H

/* Thread-local storage declarations with various attributes */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Extern TLS variable declaration */
extern __thread int tls_extern_var;

/* Weak TLS variable declaration */
extern __thread int tls_weak_var __attribute__((weak));

/* Function prototype */
int process_tls_data(int value);

/* Conditional compilation for path variation */
#ifdef USE_HIDDEN
#define VISIBILITY_ATTR __attribute__((visibility("hidden")))
#else
#define VISIBILITY_ATTR __attribute__((visibility("default")))
#endif

#endif /* TLS_H */
