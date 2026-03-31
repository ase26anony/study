/* Main test file that uses various TLS variables with different attributes */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>

/* DECL_PRESERVE_P: marked as used to prevent elimination */
__thread int tls_preserve __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable */
__thread int tls_public = 100;

/* DECL_COMMON: tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 200;

/* DECL_VISIBILITY_SPECIFIED: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 400;

/* Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* Internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* Static TLS variable (not TREE_PUBLIC) */
static __thread int tls_static = 700;

/* DECL_EXTERNAL: extern declaration */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: dllimport on supported targets */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    tls_preserve++;
    tls_public++;
    tls_common++;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    tls_protected++;
    tls_internal++;
    tls_static++;
    
    /* Use in complex expression to ensure processing */
    int* addr = &tls_public;
    *addr += tls_static;
    
    /* Use in inline asm to prevent optimization */
    asm volatile("" : "+m" (tls_common));
}

/* Function with local scope TLS variable (tests DECL_CONTEXT) */
static void function_with_local_tls(void) {
    static __thread int local_tls = 800;
    local_tls++;
    asm volatile("" : "+m" (local_tls));
}

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 50;
    
    /* Use all TLS variables */
    use_tls_variables();
    function_with_local_tls();
    
    /* Calculate sum for observable output */
    int sum = tls_preserve + tls_public + tls_common + tls_weak +
              tls_hidden + tls_default + tls_protected + tls_internal +
              tls_static;
    
    /* Use extern TLS variable if available */
    sum += tls_external;
    
    printf("TLS sum: %d\n", sum);
    
    /* Take addresses to force TLS processing */
    void* addrs[] = {
        &tls_preserve, &tls_public, &tls_common, &tls_weak,
        &tls_hidden, &tls_default, &tls_protected, &tls_internal,
        &tls_static
    };
    
    /* Use addresses to prevent dead code elimination */
    asm volatile("" : : "r"(addrs) : "memory");
    
    return 0;
}
