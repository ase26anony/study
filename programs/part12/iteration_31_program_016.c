#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// Declare TLS variables with various attributes
// These will be extern declarations in the header

// 1. Used attribute
extern __thread int tls_used_var __attribute__((used));

// 2. Weak attribute
extern __thread int tls_weak_var __attribute__((weak));

// 3. Hidden visibility
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

// 4. Default visibility (explicit)
extern __thread int tls_default_var __attribute__((visibility("default")));

// 5. DLL import style (simulated with weak)
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#else
extern __thread int tls_dllimport_var __attribute__((weak));
#endif

// 6. Common linkage variable
extern __thread int tls_common_var;

// 7. External only (no definition elsewhere)
extern __thread int tls_external_only_var;

// 8. Public variable
extern __thread int tls_public_var;

// 9. Struct type for variety
struct tls_struct {
    int a;
    double b;
    void* c;
};

extern __thread struct tls_struct tls_struct_var __attribute__((used));

// 10. Different type: double
extern __thread double tls_double_var __attribute__((visibility("hidden")));

// Function declarations
void init_tls_vars(void);
unsigned long compute_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_address(int index);

// Enum to identify TLS variables
enum TLS_VAR_ID {
    TLS_USED,
    TLS_WEAK,
    TLS_HIDDEN,
    TLS_DEFAULT,
    TLS_DLLIMPORT,
    TLS_COMMON,
    TLS_EXTERNAL,
    TLS_PUBLIC,
    TLS_STRUCT,
    TLS_DOUBLE,
    TLS_COUNT
};

#endif // TLS_VARS_H
