#ifndef TLS_PUBLIC_H
#define TLS_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public TLS variable declarations */
extern __thread int public_tls_var;
extern __thread int weak_tls_var;
extern __thread int hidden_tls_var;
extern __thread int protected_tls_var;
extern __thread int used_tls_var;

/* Common symbol test - tentative declaration */
extern __thread int common_tls_var;

/* Platform-specific attributes */
#ifdef _WIN32
extern __thread int imported_tls_var __attribute__((dllimport));
#endif

/* Function prototypes */
void init_tls_vars(void);
int compute_checksum(void);
void* thread_func(void* arg);

#ifdef __cplusplus
}
#endif

#endif /* TLS_PUBLIC_H */
