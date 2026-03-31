/* tls_decl.c - Declaration of TLS variable with various attributes */

/* Force emulated TLS */
#pragma GCC tls_model emulated

/* Declare the TLS variable with various attributes */
#ifdef _WIN32
/* Windows-specific attributes */
__declspec(dllimport) extern __thread int emulated_tls_var __attribute__((weak, visibility("default")));
#else
/* POSIX attributes */
extern __thread int emuted_tls_var __attribute__((weak, visibility("hidden")));
#endif

/* Another TLS variable for testing common linkage */
extern __thread int common_tls_var __attribute__((common));

/* Function prototype */
void use_tls_variables(void);
