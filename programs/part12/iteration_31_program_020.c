#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Extern declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;  // Just extern, no definition in header

// DLL import style attribute (simulated for non-Windows)
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
#else
    #define DLL_IMPORT __attribute__((dllimport))
#endif
extern DLL_IMPORT __thread int tls_dllimport_var;

// Struct type TLS variable
struct tls_struct {
    int a;
    double b;
    void* c;
};
extern __thread struct tls_struct tls_struct_var __attribute__((used));

// Function declarations
void init_tls_vars(int seed);
size_t compute_tls_checksum(void);
void modify_tls_vars_loop(int iterations, int seed);

// Volatile function to prevent optimization
volatile int read_tls_used_var(void);
volatile int read_tls_weak_var(void);

#endif // TLS_VARS_H
