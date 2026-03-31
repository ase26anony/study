#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

/* Declare TLS variables with various attributes that need to be copied */

/* Weak TLS variable with used attribute */
extern __thread int tls_weak_used __attribute__((weak, used));

/* TLS with hidden visibility */
extern __thread double tls_hidden __attribute__((visibility("hidden")));

/* TLS with default visibility specified */
extern __thread int tls_default_vis __attribute__((visibility("default")));

/* External TLS declaration (no definition in header) */
extern __thread volatile long tls_external;

/* Common TLS variable */
extern __thread struct {
    int a;
    double b;
} tls_common __attribute__((common));

/* DLL import style attribute simulation */
#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT __attribute__((dllimport))
#endif
extern __thread char tls_dllimport DLLIMPORT;

/* Weak external with visibility */
extern __thread float tls_weak_hidden __attribute__((weak, visibility("hidden")));

/* Public TLS variable */
extern __thread int tls_public;

/* Function prototypes */
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);
void* get_tls_address(int index);

/* Enum to identify TLS variables */
enum TLS_VAR_ID {
    TLS_WEAK_USED,
    TLS_HIDDEN,
    TLS_DEFAULT_VIS,
    TLS_EXTERNAL,
    TLS_COMMON,
    TLS_DLLIMPORT,
    TLS_WEAK_HIDDEN,
    TLS_PUBLIC,
    TLS_COUNT
};

#endif /* TLS_VARS_H */
