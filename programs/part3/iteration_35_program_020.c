/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by using non-TLS target */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS inside function scope */
void function_with_tls(void) {
    __thread int tls_function_local = 600;
    tls_function_local++;  /* Ensure TREE_USED */
}

/* External TLS declaration (DECL_EXTERNAL will be set in other file) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: Windows-specific */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Reference all TLS variables to ensure TREE_USED */
void reference_all_tls(void) {
    /* Force usage of each variable */
    tls_used += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common = 123;
    tls_weak += 4;
    tls_hidden += 5;
    tls_default += 6;
    tls_external += 7;
    tls_dllimport += 8;
}

int main(void) {
    function_with_tls();
    reference_all_tls();
    
    /* Additional references to ensure variables are marked used */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    
    return sum - sum;  /* Return 0 */
}
