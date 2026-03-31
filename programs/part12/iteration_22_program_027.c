/* tls_def.c - Defines TLS variables with different attributes */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Static TLS inside a function (has function as DECL_CONTEXT) */
static int get_local_tls(void) {
    static __thread int local_func_tls = 500;
    return local_func_tls++;
}

/* TLS with non-constant initializer */
int get_init_value(void) { return 999; }
__thread int dynamic_init_tls = 0; /* Will be initialized at runtime */

/* Function that uses and returns TLS addresses */
void* get_public_tls_addr(void) {
    return &public_tls;
}

void* get_weak_tls_addr(void) {
    return &weak_tls_var;
}

int get_and_increment_local(void) {
    return get_local_tls();
}

void init_dynamic_tls(void) {
    dynamic_init_tls = get_init_value();
}

/* Common TLS (uninitialized global) */
__thread int common_tls;

/* DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* On non-Windows, simulate with extern */
extern __thread int imported_tls;
#endif
