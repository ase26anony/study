/* tls_decl.h - Declarations for TLS variable with various attributes */

#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Visibility attribute - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
#ifdef _WIN32
#  ifdef BUILDING_DLL
#    define DLLEXPORT __declspec(dllexport)
#  else
#    define DLLEXPORT __declspec(dllimport)
#  endif
#  define VISIBILITY_DEFAULT
#else
#  define DLLEXPORT
#  define VISIBILITY_DEFAULT __attribute__((visibility("default")))
#endif

/* Weak attribute - sets DECL_WEAK */
#ifdef __GNUC__
#  define WEAK_ATTR __attribute__((weak))
#else
#  define WEAK_ATTR
#endif

/* External declaration with weak linkage, visibility, and DLL import */
extern DLLEXPORT VISIBILITY_DEFAULT WEAK_ATTR _Thread_local int tls_var;

/* Function to access the TLS variable */
int get_tls_value(void);
void set_tls_value(int val);

#ifdef __cplusplus
}
#endif

#endif /* TLS_DECL_H */
