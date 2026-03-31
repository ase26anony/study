/* tls_decl.h - Declarations for TLS variable with various attributes */

#ifndef TLS_DECL_H
#define TLS_DECL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Visibility attribute */
#ifdef _WIN32
#  ifdef BUILDING_DLL
#    define DLL_EXPORT __declspec(dllexport)
#  else
#    define DLL_EXPORT __declspec(dllimport)
#  endif
#  define VISIBILITY_DEFAULT
#else
#  define DLL_EXPORT
#  define VISIBILITY_DEFAULT __attribute__((visibility("default")))
#endif

/* Declare TLS variable with multiple attributes */
/* This declaration will be weak and potentially dllimport on Windows */
extern DLL_EXPORT VISIBILITY_DEFAULT __thread int emulated_tls_var 
    __attribute__((weak));

/* Function to use the TLS variable */
int use_tls_variable(void);

#ifdef __cplusplus
}
#endif

#endif /* TLS_DECL_H */
