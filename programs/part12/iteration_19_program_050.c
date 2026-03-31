#ifndef TLS_H
#define TLS_H

/* Visibility attributes for DECL_VISIBILITY testing */
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
extern DLL_IMPORT EMU_TLS int external_tls;

/* Weak external reference */
extern EMU_TLS int weak_tls __attribute__((weak));

/* Function prototypes */
int get_checksum(void);
void use_tls_addresses(void);

/* Opaque function to prevent optimization */
void opaque_function(void* ptr) __attribute__((noipa));

#endif /* TLS_H */
