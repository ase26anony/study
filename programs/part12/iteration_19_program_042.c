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
#if __GNUC__ >= 4
#define EMU_TLS __thread __attribute__((tls_model("emulated")))
#else
#define EMU_TLS __thread
#endif

/* External TLS with visibility and DLL import */
extern DLL_IMPORT EMU_TLS int external_tls;

/* Weak external TLS */
extern EMU_TLS int weak_tls __attribute__((weak));

/* Common TLS (tentative definition) */
extern EMU_TLS int common_tls;

/* Function to take addresses (prevents optimization) */
int use_tls_addresses(void);
int compute_checksum(void);

#endif /* TLS_H */
