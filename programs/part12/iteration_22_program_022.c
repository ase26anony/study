/* tls_def.c - Defines TLS variables with different attributes */

/* Force emulated TLS even if native is available */
#ifdef __GNUC__
#  define TLS __thread
#else
#  define TLS _Thread_local
#endif

/* Public TLS with default visibility and initializer */
TLS int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol that can be overridden */
TLS int weak_tls_var __attribute__((weak)) = 100;

/* Common TLS (tentative definition) */
TLS int common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
TLS int dllimport_tls __attribute__((dllimport));
#else
/* Simulate similar concept with visibility */
TLS int dllimport_tls __attribute__((visibility("hidden")));
#endif

/* Function that returns address of TLS variable */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls(void) {
    return weak_tls_var;
}

/* Function with static TLS in local scope */
void func_with_local_tls(void) {
    static TLS int local_func_tls = 0;
    local_func_tls++;
}

/* Force preservation of TLS symbols */
void preserve_tls_symbols(void) {
    /* Take addresses to ensure symbols are preserved */
    volatile int* ptr1 = &public_tls;
    volatile int* ptr2 = &weak_tls_var;
    volatile int* ptr3 = &common_tls;
    volatile int* ptr4 = &dllimport_tls;
    (void)ptr1; (void)ptr2; (void)ptr3; (void)ptr4;
}
