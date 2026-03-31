#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern TLS declarations with various attributes
extern __thread int tls_used __attribute__((used));
extern __thread int tls_weak __attribute__((weak));
extern __thread int tls_hidden __attribute__((visibility("hidden")));
extern __thread int tls_default __attribute__((visibility("default")));
extern __thread int tls_external;  // Just extern, no definition here

// DLL import simulation (Windows-specific attribute)
#ifdef _WIN32
extern __thread int tls_dllimport __attribute__((dllimport));
#else
extern __thread int tls_dllimport __attribute__((visibility("protected")));
#endif

// Struct type TLS variable
struct tls_struct {
    int a;
    double b;
    void* ptr;
};

extern __thread struct tls_struct tls_complex __attribute__((used, weak));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void* get_tls_address(int index);

// Enum to identify TLS variables
enum tls_var_id {
    TLS_USED,
    TLS_WEAK,
    TLS_HIDDEN,
    TLS_DEFAULT,
    TLS_EXTERNAL,
    TLS_DLLIMPORT,
    TLS_COMPLEX,
    TLS_COUNT
};

#endif // TLS_VARS_H
