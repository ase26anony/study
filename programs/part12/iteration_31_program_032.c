#ifndef TLS_VARS_H
#define TLS_VARS_H

#include <stddef.h>

// TLS variables with various attributes declared as extern
// These will be defined in tls_def.c

// Used attribute sets TREE_USED
extern __thread int tls_used_var __attribute__((used));

// Weak attribute sets DECL_WEAK
extern __thread int tls_weak_var __attribute__((weak));

// Hidden visibility sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED
extern __thread int tls_hidden_var __attribute__((visibility("hidden")));

// Default visibility
extern __thread int tls_default_var __attribute__((visibility("default")));

// DLL import attribute (for Windows targets)
#ifdef _WIN32
extern __thread int tls_dllimport_var __attribute__((dllimport));
#else
extern __thread int tls_dllimport_var;
#endif

// Common linkage variable
extern __thread int tls_common_var;

// External declaration only (DECL_EXTERNAL)
extern __thread int tls_external_only_var;

// Public variable (TREE_PUBLIC)
extern __thread int tls_public_var;

// Variable with multiple attributes
extern __thread int tls_multi_attr_var __attribute__((used, weak, visibility("hidden")));

// Different types of TLS variables
extern __thread double tls_double_var;
extern __thread char tls_char_var;
extern __thread void* tls_ptr_var;

// Struct type TLS variable
struct tls_struct {
    int a;
    double b;
    char c;
};
extern __thread struct tls_struct tls_struct_var;

// Function declarations
void init_tls_vars(void);
unsigned long compute_tls_checksum(void);
void modify_tls_vars(int seed);
void* get_tls_address(int index, int seed);

#endif // TLS_VARS_H
