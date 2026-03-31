/* Test program for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with used attribute (tests DECL_PRESERVE_P, TREE_USED) */
__thread int tls_used __attribute__((used)) = 42;

/* Test 2: Public TLS with default visibility (tests TREE_PUBLIC, DECL_VISIBILITY) */
__thread int tls_public __attribute__((visibility("default")));

/* Test 3: Weak TLS symbol (tests DECL_WEAK) */
__thread int tls_weak __attribute__((weak)) = 100;

/* Test 4: Hidden visibility TLS (tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED) */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* Test 5: Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected")));

/* Test 6: Common TLS (tests DECL_COMMON) - tentative definition */
__thread int tls_common;

/* Test 7: Static TLS with internal visibility (tests DECL_CONTEXT from function scope) */
static void func_with_tls(void) {
    static __thread int tls_in_func = 7;
    tls_in_func++;
}

/* Test 8: External declaration (tests DECL_EXTERNAL) - defined in emutls_aux.c */
extern __thread int tls_external;

/* Test 9: DLL import simulation (tests DECL_DLLIMPORT_P) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
/* Simulate DLL import with weak external */
__thread int tls_dllimport __attribute__((weak));
#endif

/* Test 10: Multiple combined attributes */
__thread int tls_combined __attribute__((used, visibility("hidden"), weak));

/* Function to ensure TLS variables are referenced */
void reference_tls_vars(void) {
    /* Take addresses to prevent optimization */
    volatile int *ptr;
    
    ptr = &tls_used;
    tls_used += 1;
    
    ptr = &tls_public;
    tls_public = 123;
    
    ptr = &tls_weak;
    tls_weak = 456;
    
    ptr = &tls_hidden;
    tls_hidden = 789;
    
    ptr = &tls_protected;
    tls_protected = 101;
    
    ptr = &tls_common;
    tls_common = 202;
    
    func_with_tls();
    
    ptr = &tls_external;
    tls_external = 303;
    
    ptr = &tls_dllimport;
    tls_dllimport = 404;
    
    ptr = &tls_combined;
    tls_combined = 505;
    
    /* Use inline asm to ensure variables are marked used */
    __asm__ volatile ("" : : "r"(&tls_used));
    __asm__ volatile ("" : : "r"(&tls_public));
    __asm__ volatile ("" : : "r"(&tls_hidden));
}

/* Main function */
int main(void) {
    reference_tls_vars();
    
    /* Simple validation */
    if (tls_used != 43) return 1;
    if (tls_public != 123) return 2;
    if (tls_weak != 456) return 3;
    
    return 0;
}

#ifdef __cplusplus
}
#endif
