#ifndef TLS_EMULATION_H
#define TLS_EMULATION_H

#include <stddef.h>

/* Force emulated TLS by using a non-TLS target if needed */
#ifdef __GNUC__
  #define TLS_ATTR __thread
  #define WEAK_ATTR __attribute__((weak))
  #define USED_ATTR __attribute__((used))
  #define HIDDEN_ATTR __attribute__((visibility("hidden")))
  #define DEFAULT_VIS_ATTR __attribute__((visibility("default")))
  #ifdef _WIN32
    #define DLLIMPORT_ATTR __declspec(dllimport)
  #else
    #define DLLIMPORT_ATTR
  #endif
  #define NOINLINE_ATTR __attribute__((noinline))
  #define NOIPA_ATTR __attribute__((noipa))
#else
  #define TLS_ATTR _Thread_local
  #define WEAK_ATTR
  #define USED_ATTR
  #define HIDDEN_ATTR
  #define DEFAULT_VIS_ATTR
  #define DLLIMPORT_ATTR
  #define NOINLINE_ATTR
  #define NOIPA_ATTR
#endif

/* External TLS declaration with visibility and DLL import attributes */
extern TLS_ATTR DLLIMPORT_ATTR DEFAULT_VIS_ATTR int external_tls;

/* Function prototypes */
NOINLINE_ATTR int use_global_tls(void);
NOINLINE_ATTR int use_static_tls(void);
NOINLINE_ATTR int use_external_tls(void);
NOIPA_ATTR void opaque_function(void* ptr);

#endif /* TLS_EMULATION_H */
