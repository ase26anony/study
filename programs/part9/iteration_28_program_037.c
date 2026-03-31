/* tls_decl.c - Declaration of TLS variable with various attributes */

/* Force emulated TLS */
#pragma GCC tls_model emulated

/* Declare as weak external TLS variable with hidden visibility */
#ifdef _WIN32
__declspec(dllimport) __declspec(thread)
#else
__attribute__((weak, visibility("hidden")))
#endif
extern int emulated_tls_var;

/* Function prototype that will use the TLS variable */
int get_tls_value(void);
