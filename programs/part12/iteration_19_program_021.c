/* tls.h - Header file for TLS declarations */
#ifndef TLS_H
#define TLS_H

/* Visibility attributes */
#define HIDDEN __attribute__((visibility("hidden")))
#define DEFAULT_VIS __attribute__((visibility("default")))

/* Weak linkage */
#define WEAK_SYMBOL __attribute__((weak))

/* Used attribute to ensure preservation */
#define FORCE_USED __attribute__((used))

/* DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External TLS declarations with various attributes */
extern __thread int external_tls DEFAULT_VIS;
extern __thread int weak_tls WEAK_SYMBOL;
extern DLL_IMPORT __thread int dllimport_tls;

/* Function prototypes */
int get_checksum(void);
void use_tls_addresses(void);

#endif /* TLS_H */
