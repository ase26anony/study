#ifndef TLS_H
#define TLS_H

/* Visibility attributes for testing DECL_VISIBILITY */
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

/* External TLS declaration with visibility and DLL import attributes */
extern DLL_IMPORT EMU_TLS int external_tls 
    __attribute__((visibility("hidden"), weak));

/* Common linkage test */
extern EMU_TLS int common_tls;

/* Function prototypes */
int get_checksum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noinline, used));

/* Opaque function to prevent optimization */
void opaque_func(void*) __attribute__((noipa));

#endif /* TLS_H */
