#ifndef TLS_DEFS_H
#define TLS_DEFS_H

// Visibility attributes
#define HIDDEN __attribute__((visibility("hidden")))
#define DEFAULT_VIS __attribute__((visibility("default")))

// Force preservation
#define PRESERVE __attribute__((used))

// Weak linkage
#define WEAK_LINK __attribute__((weak))

// DLL import simulation (works on most targets)
#define DLL_IMPORT __attribute__((dllimport))

// Common linkage via tentative definition
#define COMMON_LINK // No attribute needed, just tentative definition

// Thread-local storage
#ifdef __cplusplus
    #define THREAD_LOCAL thread_local
#else
    #define THREAD_LOCAL __thread
#endif

// External declarations (will be defined in other files)
extern THREAD_LOCAL int extern_tls_var;
extern THREAD_LOCAL HIDDEN int extern_hidden_tls;

// Weak external declaration
extern THREAD_LOCAL WEAK_LINK int weak_extern_tls_var;

#endif // TLS_DEFS_H
