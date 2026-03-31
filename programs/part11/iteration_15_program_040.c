/* Main test file for emulated TLS attribute coverage */
#include <stdio.h>

/* Force emulated TLS model for coverage */
#pragma GCC tls_model emulated

/* DECL_PRESERVE_P: Used attribute ensures preservation */
__thread int tls_preserve __attribute__((used)) = 42;

/* TREE_PUBLIC: Non-static (public) TLS variable */
__thread int tls_public = 100;

/* DECL_COMMON: Tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 200;

/* DECL_VISIBILITY: Hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* DECL_VISIBILITY_SPECIFIED: Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* DECL_EXTERNAL: External declaration (defined elsewhere) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: DLL import attribute (Windows targets) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function with static TLS variable (different DECL_CONTEXT) */
static void use_local_tls(void) {
    /* DECL_CONTEXT: Function-local static TLS */
    static __thread int tls_local_static = 500;
    tls_local_static++;
}

/* Complex usage to ensure TREE_USED is set */
void complex_tls_usage(void) {
    /* Take address, use in expressions */
    int *ptr = &tls_public;
    tls_common = tls_preserve + *ptr;
    
    /* Use weak variable */
    if (&tls_weak) {
        tls_hidden = tls_weak * 2;
    }
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_protected));
    
    use_local_tls();
}

int main(void) {
    int sum = 0;
    
    /* Ensure all TLS variables are used (sets TREE_USED) */
    sum += tls_preserve;
    sum += tls_public;
    sum += tls_common;
    
    if (&tls_weak) {
        sum += tls_weak;
    }
    
    sum += tls_hidden;
    sum += tls_protected;
    
    /* Use external variable */
    sum += tls_external;
    
    #if defined(_WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
    sum += tls_dllimport;
    #endif
    
    complex_tls_usage();
    
    printf("TLS sum: %d\n", sum);
    return 0;
}
