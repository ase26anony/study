/* tls_decl.c - External declarations for TLS variable */

/* Force emulated TLS */
#pragma GCC tls_model emulated

/* Declare the TLS variable with various attributes */
#ifdef _WIN32
/* Windows-specific attributes */
__declspec(dllimport) extern __thread int emutls_var __attribute__((weak, visibility("default")));
#else
/* POSIX attributes */
extern __thread int emutls_var __attribute__((weak, visibility("hidden")));
#endif

/* Function prototype that will use the TLS variable */
int get_emutls_value(void);
