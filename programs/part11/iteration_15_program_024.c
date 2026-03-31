/* Test for emulated TLS attribute copying - Main file */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>

/* DECL_PRESERVE_P: Used attribute ensures preservation */
__thread int tls_preserved __attribute__((used)) = 42;

/* TREE_PUBLIC: Public TLS variable (non-static) */
__thread int tls_public = 100;

/* DECL_COMMON: Tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 200;

/* DECL_VISIBILITY: Hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* DECL_VISIBILITY_SPECIFIED: Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* Internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 500;

/* DECL_CONTEXT: File scope context */
static __thread int tls_static_context = 600;

/* TREE_USED: Ensure variables are used */
void use_tls_variables(void) {
    tls_preserved += 1;
    tls_public += 2;
    tls_common = tls_weak + tls_hidden;
    tls_protected *= 2;
    tls_internal -= 50;
    tls_static_context = 999;
    
    /* Take addresses to ensure processing */
    volatile int *ptr1 = &tls_preserved;
    volatile int *ptr2 = &tls_public;
    (void)ptr1;
    (void)ptr2;
}

/* DECL_EXTERNAL: External declaration */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: DLL import (conditional) */
#ifdef _WIN32
extern __thread int tls_imported __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_imported __attribute__((dllimport));
#endif

/* Complex usage to force emulation */
__thread int tls_complex[10];

void complex_tls_usage(void) {
    for (int i = 0; i < 10; i++) {
        tls_complex[i] = i * i;
    }
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_complex[0]) : "memory");
}

/* Function with local TLS context */
static void function_with_local_tls(void) {
    /* DECL_CONTEXT: Function local scope */
    static __thread int tls_local_context = 700;
    tls_local_context++;
}

int main(void) {
    use_tls_variables();
    function_with_local_tls();
    complex_tls_usage();
    
    /* Initialize common if not already */
    if (tls_common == 0) {
        tls_common = 1234;
    }
    
    /* Use external */
    tls_external = 888;
    
    /* Sum all TLS values for observable output */
    int sum = tls_preserved + tls_public + tls_common + 
              tls_weak + tls_hidden + tls_protected + 
              tls_internal + tls_static_context + tls_external;
    
    for (int i = 0; i < 10; i++) {
        sum += tls_complex[i];
    }
    
    printf("TLS sum: %d\n", sum);
    return 0;
}
