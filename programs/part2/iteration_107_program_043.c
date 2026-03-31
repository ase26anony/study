/* File: tls_public.h - Common declarations */
#ifndef TLS_PUBLIC_H
#define TLS_PUBLIC_H

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Forward declarations for TLS variables defined elsewhere */
extern __thread int external_tls_var;
extern __thread int common_tls_var;
extern __thread int weak_tls_var;
extern DLL_IMPORT __thread int imported_tls_var;

#endif
