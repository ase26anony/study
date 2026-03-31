/* Test for emulated TLS attribute copying - main file */

/* Force emulated TLS model for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>

/* DECL_PRESERVE_P: used attribute ensures preservation */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static at file scope */
__thread int tls_public = 100;

/* DECL_COMMON: tentative definition (no initializer) */
__thread int tls_common;

/* DECL_WEAK: weak symbol */
__thread int tls_weak __attribute__((weak)) = 200;

/* DECL_VISIBILITY_SPECIFIED: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 400;

/* Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* Internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* Static TLS (not TREE_PUBLIC) */
static __thread int tls_static = 700;

/* DECL_EXTERNAL: extern declaration (defined in another file) */
extern __thread int tls_extern;

/* For DLL import testing on supported targets */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function to ensure TREE_USED is set */
void use_tls_vars(void) {
    /* Reference all TLS variables to mark them as used */
    tls_used += 1;
    tls_public += 2;
    tls_common = 50;
    tls_weak += 3;
    tls_hidden += 4;
    tls_default += 5;
    tls_protected += 6;
    tls_internal += 7;
    tls_static += 8;
    
    /* External variables */
    if (tls_extern) {
        tls_extern += 9;
    }
    
#ifdef tls_dllimport
    if (tls_dllimport) {
        tls_dllimport += 10;
    }
#endif
}

/* Another function with its own static TLS */
static void local_function(void) {
    /* DECL_CONTEXT: TLS in function scope */
    static __thread int tls_in_function = 800;
    tls_in_function++;
}

/* Take address to prevent optimizations */
void take_addresses(void) {
    void *addrs[] = {
        &tls_used,
        &tls_public,
        &tls_common,
        &tls_weak,
        &tls_hidden,
        &tls_default,
        &tls_protected,
        &tls_internal,
        &tls_static,
        &tls_extern,
#ifdef tls_dllimport
        &tls_dllimport,
#endif
    };
    
    /* Use in inline asm to ensure processing */
    __asm__ volatile ("" : : "r"(addrs) : "memory");
}

int main(void) {
    int sum = 0;
    
    /* Initialize and use TLS variables */
    use_tls_vars();
    local_function();
    take_addresses();
    
    /* Calculate sum for observable behavior */
    sum += tls_used;
    sum += tls_public;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_static;
    sum += tls_extern;
    
#ifdef tls_dllimport
    sum += tls_dllimport;
#endif
    
    printf("TLS sum: %d\n", sum);
    return 0;
}
