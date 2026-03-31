#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

/* TLS variable with multiple attributes to be copied */
extern __thread int tls_used_weak __attribute__((used, weak));

/* TLS with visibility attributes */
extern __thread double tls_hidden __attribute__((visibility("hidden")));
extern __thread double tls_default __attribute__((visibility("default")));

/* External TLS declaration (DECL_EXTERNAL will be true) */
extern __thread volatile long tls_external;

/* TLS with DLL import attribute simulation */
#ifdef _WIN32
extern __thread char tls_dllimport __attribute__((dllimport));
#else
extern __thread char tls_dllimport;
#endif

/* TLS struct type to test complex types */
struct tls_struct {
    int a;
    double b;
    void* c;
};

/* TLS struct with attributes */
extern __thread struct tls_struct tls_complex __attribute__((used));

/* Static TLS in header (internal linkage) */
static __thread unsigned int tls_static_internal = 42;

/* Function declarations */
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void* get_tls_address(int index, int modify);
void __attribute__((noinline)) modify_tls_via_ptr(void* ptr, int value);
int __attribute__((noinline)) read_tls_via_ptr(const void* ptr);

#endif /* TLS_VARS_H */
