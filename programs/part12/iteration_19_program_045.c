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

/* External TLS declarations with various attributes */
extern EMU_TLS int external_tls_default 
    __attribute__((weak, visibility("default")));
    
extern EMU_TLS int external_tls_hidden 
    __attribute__((visibility("hidden")));
    
extern DLL_IMPORT EMU_TLS int external_tls_dllimport;

/* Opaque function to prevent optimization */
int opaque_use(void* ptr) __attribute__((noipa));

#endif /* TLS_H */
