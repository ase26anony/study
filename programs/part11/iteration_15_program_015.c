/* Main test file with diverse TLS declarations */

/* Force emulated TLS */
#pragma GCC tls_model emulated

#include <stdio.h>

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC + DECL_COMMON - tentative definition (common symbol) */
__thread int tls_common;

/* TREE_PUBLIC + initialized */
__thread int tls_public = 100;

/* Static TLS (not TREE_PUBLIC) */
static __thread int tls_static = 200;

/* DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED - hidden */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED - protected */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* DECL_VISIBILITY_SPECIFIED - internal */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* Function-scoped TLS (tests DECL_CONTEXT) */
void test_function_scope(void) {
    static __thread int tls_func_scope = 700;
    tls_func_scope++;
}

/* Extern declaration (will be defined in another file) */
extern __thread int tls_extern;

/* DLL import simulation (if supported) */
#ifdef _WIN32
__declspec(dllimport) extern __thread int tls_dllimport;
#else
/* Use dllimport-like attribute on non-Windows if supported */
#if __has_attribute(dllimport)
extern __thread int tls_dllimport __attribute__((dllimport));
#else
extern __thread int tls_dllimport;
#endif
#endif

/* Complex usage to ensure TREE_USED is set */
void use_all_tls(void) {
    /* Reference all TLS variables */
    tls_used += 1;
    tls_common = tls_public + 1;
    tls_static = tls_weak + 1;
    tls_hidden = tls_protected + 1;
    tls_internal = tls_extern + 1;
    
    /* Take addresses (complex usage) */
    int *ptr1 = &tls_used;
    int *ptr2 = &tls_common;
    
    /* Use in inline asm to prevent optimization */
    __asm__ volatile ("" : "+m" (tls_public));
    
    test_function_scope();
}

int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    tls_common = 50;
    tls_public = 150;
    tls_static = 250;
    tls_weak = 350;
    tls_hidden = 450;
    tls_protected = 550;
    tls_internal = 650;
    
    /* Use the extern variable */
    sum += tls_extern;
    
    /* Use all variables */
    use_all_tls();
    
    /* Calculate sum */
    sum += tls_used;
    sum += tls_common;
    sum += tls_public;
    sum += tls_static;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    
    printf("TLS sum: %d\n", sum);
    
    /* Reference DLL import variable if available */
#ifdef TLS_DLLIMPORT_AVAILABLE
    sum += tls_dllimport;
#endif
    
    return sum > 0 ? 0 : 1;
}
