/* Test for EMUTLS attribute copying - C file with various TLS attributes */

/* Force EMUTLS by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void use_local_tls(void) {
    /* Local TLS variable - different context */
    static __thread int tls_local_scope = 600;
    tls_local_scope++;
}

/* DECL_EXTERNAL: external TLS declaration (defined in another file) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: Windows-specific - we'll use a macro to make it portable */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* On non-Windows, we'll use a different attribute or just regular TLS */
__thread int tls_dllimport = 700;
#endif

/* Function to ensure TREE_USED is set for all variables */
void use_all_tls_variables(void) {
    /* Reference all TLS variables to mark them as TREE_USED */
    tls_used += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common = 123;
    tls_weak += 4;
    tls_hidden += 5;
    tls_default += 6;
    tls_external += 7;  /* External reference */
    tls_dllimport += 8;
    
    /* Call function with local TLS */
    use_local_tls();
}

/* Main function for C test */
int main_c(void) {
    use_all_tls_variables();
    
    /* Additional uses to ensure optimization doesn't remove them */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_external;
    sum += tls_dllimport;
    
    return sum != 0 ? 0 : 1;
}
