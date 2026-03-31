/* tls_def.c - Defines TLS variables with different attributes */

/* Public TLS with default visibility */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside a function context */
static void foo(void) {
    static __thread int local_tls = 100;
    local_tls++;
}

/* Common TLS (uninitialized, may become common) */
__thread int common_tls;

/* DLL import style attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate DLL import with visibility */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses TLS and returns address */
int* get_public_tls_addr(void) {
    /* Ensure TREE_USED is set */
    public_tls++;
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls(void) {
    if (&weak_tls_var != NULL) {
        weak_tls_var = 99;
        return weak_tls_var;
    }
    return 0;
}

/* Function that uses local TLS */
int use_local_tls(void) {
    foo();
    return 0;
}

/* Function with non-constant initializer simulation */
int get_random_value(void) {
    return 123; /* Not truly random, but non-constant from compiler's view */
}

/* TLS with non-constant initializer */
__thread int dynamic_init_tls = 123; /* Will be initialized at runtime */

/* Preserve this symbol */
__thread int preserved_tls __attribute__((used)) = 456;
