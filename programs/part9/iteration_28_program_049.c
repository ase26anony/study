/* tls_decl.h - Declarations for TLS variable with various attributes */

#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Visibility attribute - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
#ifdef _WIN32
#  ifdef BUILDING_DLL
#    define DLLEXPORT __declspec(dllexport)
#  else
#    define DLLEXPORT __declspec(dllimport)
#  endif
#  define VISIBILITY
#else
#  define DLLEXPORT
#  define VISIBILITY __attribute__((visibility("hidden")))
#endif

/* Weak attribute - triggers DECL_WEAK */
#ifdef __GNUC__
#  define WEAK __attribute__((weak))
#else
#  define WEAK
#endif

/* Common attribute - triggers DECL_COMMON */
#ifdef __GNUC__
#  define COMMON __attribute__((common))
#else
#  define COMMON
#endif

/* External declaration with multiple attributes */
extern DLLEXPORT VISIBILITY WEAK COMMON __thread int emulated_tls_var;

/* Function to use the TLS variable */
int use_tls_variable(void);

#ifdef __cplusplus
}
#endif

#endif /* TLS_DECL_H */
