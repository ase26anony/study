#ifndef TLS_COMMON_H
#define TLS_COMMON_H

/* Attributes to trigger specific declaration properties */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Visibility attribute for DECL_VISIBILITY_SPECIFIED */
#define EXPORTED __attribute__((visibility("default")))

/* Weak attribute for DECL_WEAK */
#define WEAK_SYMBOL __attribute__((weak))

/* Common attribute for DECL_COMMON */
#define COMMON_SYMBOL __attribute__((common))

/* External TLS declaration - will be defined in tls.c */
extern __thread int tls_extern_var;

/* Function prototype */
void tls_operations(int value);

#endif /* TLS_COMMON_H */
