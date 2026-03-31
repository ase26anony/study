#ifndef TLS_H
#define TLS_H

/* Visibility attributes for testing */
#ifdef _WIN32
  #define DLL_IMPORT __declspec(dllimport)
  #define DLL_EXPORT __declspec(dllexport)
#else
  #define DLL_IMPORT __attribute__((dllimport))
  #define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* External TLS declaration with attributes */
extern __thread int external_tls 
    __attribute__((weak, visibility("default")));

/* Another with hidden visibility */
extern __thread int hidden_tls 
    __attribute__((visibility("hidden")));

/* DLL import simulation */
DLL_IMPORT extern __thread int imported_tls;

/* Function prototypes */
int get_checksum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noinline, used));

#endif /* TLS_H */
