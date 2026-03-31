/* Main test file for emulated TLS attribute coverage */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS model for testing */
#pragma GCC tls_model emulated

/* DECL_PRESERVE_P: marked as used to prevent elimination */
__thread int tls_preserve __attribute__((used)) = 42;

/* TREE_PUBLIC: public TLS variable with initialization */
__thread int tls_public = 100;

/* DECL_COMMON: tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 200;

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* DECL_VISIBILITY_SPECIFIED: protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* DECL_VISIBILITY_SPECIFIED: internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 500;

/* Static TLS variable (not TREE_PUBLIC) */
static __thread int tls_static = 600;

/* DECL_EXTERNAL: external declaration */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: dllimport on supported targets */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    tls_preserve += 1;
    tls_public += 2;
    tls_common += 3;
    tls_weak += 4;
    tls_hidden += 5;
    tls_protected += 6;
    tls_internal += 7;
    tls_static += 8;
    
    /* External variables */
    if (&tls_external) {
        tls_external += 9;
    }
    
#ifdef _WIN32
    if (&tls_dllimport) {
        tls_dllimport += 10;
    }
#endif
}

/* Function with static scope TLS variable (tests DECL_CONTEXT) */
static void function_with_tls(void) {
    static __thread int tls_in_function = 700;
    tls_in_function += 11;
}

/* Take address of TLS variables (complex usage pattern) */
void take_addresses(void) {
    void* addrs[] = {
        &tls_preserve,
        &tls_public,
        &tls_common,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_internal,
        &tls_static,
        &tls_external,
    };
    
    /* Use addresses in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(addrs) : "memory");
}

int main(void) {
    int sum = 0;
    
    /* Initialize common TLS variable */
    tls_common = 800;
    
    /* Use all TLS variables */
    use_tls_variables();
    function_with_tls();
    take_addresses();
    
    /* Calculate sum for verification */
    sum = tls_preserve + tls_public + tls_common + tls_weak +
          tls_hidden + tls_protected + tls_internal + tls_static;
    
    printf("TLS sum: %d\n", sum);
    
    return 0;
}
