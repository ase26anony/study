/* tls_decl.c - Declaration of TLS variable with various attributes */

/* Force emulated TLS model */
#pragma GCC tls_model emulated

/* Declare the TLS variable with various attributes */
#ifdef _WIN32
/* Windows-specific attributes */
__declspec(dllimport) extern __declspec(thread) 
#else
/* POSIX attributes */
extern __thread __attribute__((weak, visibility("hidden")))
#endif
int emulated_tls_var;

/* Another TLS variable for common attribute testing */
extern __thread __attribute__((common)) int common_tls_var;

/* Function prototype */
void use_tls_variables(void);
