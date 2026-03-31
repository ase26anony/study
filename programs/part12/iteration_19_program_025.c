#ifndef TLS_H
#define TLS_H

/* Visibility attributes */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* Force emulated TLS */
#if defined(__GNUC__) && !defined(__clang__)
#define EMU_TLS __thread __attribute__((tls_model("emulated")))
#else
#define EMU_TLS __thread
#endif

/* External TLS declaration with visibility */
extern EMU_TLS int external_tls 
    __attribute__((weak, visibility("default")));

/* DLL import simulation */
extern DLL_IMPORT EMU_TLS int imported_tls;

/* Common linkage test */
extern EMU_TLS int common_tls;

/* Opaque function to prevent optimization */
int opaque_func(int* ptr) __attribute__((noipa));

#endif /* TLS_H */
