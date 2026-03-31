#ifndef TLS_DEFS_H
#define TLS_DEFS_H

/* Visibility attributes */
#define HIDDEN __attribute__((visibility("hidden")))
#define DEFAULT_VIS __attribute__((visibility("default")))

/* Linkage attributes */
#define WEAK_SYM __attribute__((weak))
#define USED_SYM __attribute__((used))

/* DLL import simulation */
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_IMPORT __attribute__((dllimport))
    #define DLL_EXPORT __attribute__((dllexport))
#endif

/* Prevent optimization */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

#endif /* TLS_DEFS_H */
