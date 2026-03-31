#ifndef TLS_VARS_H
#define TLS_VARS_H

// Extern TLS declarations with various attributes
extern __thread int tls_used_var __attribute__((used));
extern __thread int tls_weak_var __attribute__((weak));
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));
extern __thread int tls_default_var __attribute__((visibility("default")));
extern __thread int tls_extern_var;  // Just extern, no definition in header

// DLL import simulation (for DECL_DLLIMPORT_P)
#ifdef _WIN32
extern __declspec(dllimport) __thread int tls_dllimport_var;
#else
// Simulate with weak visibility on non-Windows
extern __thread int tls_dllimport_var __attribute__((weak));
#endif

// Different types
extern __thread double tls_double_var;
extern __thread struct Point {
    int x;
    int y;
} tls_struct_var;

// Static TLS in header (forces different context)
static __thread int tls_static_in_header __attribute__((used));

// Function declarations
void init_tls_vars(void);
unsigned long compute_checksum(void);
void modify_tls_vars(int idx);
void* get_tls_address(int var_id);

#endif // TLS_VARS_H
