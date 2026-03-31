#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Visibility attributes */
#define HIDDEN __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define INTERNAL __attribute__((visibility("internal")))

/* Weak attribute */
#define WEAK_SYMBOL __attribute__((weak))

/* Used attribute to prevent elimination */
#define FORCE_USED __attribute__((used))

/* TLS model attribute */
#define GLOBAL_DYNAMIC_TLS __attribute__((tls_model("global-dynamic")))

#endif
