#ifndef TLS_COMMON_H
#define TLS_COMMON_H

#include <stdio.h>

/* Visibility attributes */
#if defined(__GNUC__) && !defined(_WIN32)
#define HIDDEN __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define WEAK __attribute__((weak))
#define USED __attribute__((used))
#else
#define HIDDEN
#define PROTECTED
#define WEAK
#define USED
#endif

/* DLL import for Windows */
#ifdef _WIN32
#define DLLIMPORT __attribute__((dllimport))
#else
#define DLLIMPORT
#endif

/* Function prototypes */
void check_tls_values(void);
unsigned int compute_checksum(void);

#endif /* TLS_COMMON_H */
