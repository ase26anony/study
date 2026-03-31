/* Test program for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with used attribute - tests DECL_PRESERVE_P, TREE_USED */
__thread int tls_used __attribute__((used)) = 42;

/* Test 2: Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int tls_public __attribute__((visibility("default")));

/* Test 3: Weak TLS symbol - tests DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 100;

/* Test 4: Hidden visibility TLS - tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* Test 5: Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected")));

/* Test 6: Static TLS (non-public) for contrast */
static __thread int tls_static = 200;

/* Test 7: TLS with no initializer (common) - tests DECL_COMMON */
__thread int tls_common;

/* Test 8: External declaration (defined in aux file) - tests DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 9: Function-scoped TLS for DECL_CONTEXT */
static void test_function(void) {
    /* Local TLS variable - will have function as DECL_CONTEXT */
    static __thread int tls_local_in_func = 300;
    
    /* Use it to prevent optimization */
    asm volatile("" : : "r"(&tls_local_in_func));
}

/* Test 10: DLL import simulation (using weak alias) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, simulate with weak attribute */
extern __thread int tls_imported __attribute__((weak));
#endif

/* Forward declaration for function in aux file */
void aux_function(void);

int main(void) {
    /* Take addresses to ensure variables are referenced */
    int *ptrs[] = {
        &tls_used,
        &tls_public,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_static,
        &tls_common,
        &tls_external,
        NULL
    };
    
    /* Modify some variables */
    tls_used += 1;
    tls_public = 123;
    tls_weak = tls_weak * 2;
    tls_hidden = 456;
    tls_protected = 789;
    tls_static += 10;
    tls_common = 999;
    
    /* Call function with local TLS */
    test_function();
    
    /* Call aux function that uses external TLS */
    aux_function();
    
    /* Use inline asm to reference variables that might be optimized out */
    asm volatile("" : : "r"(&tls_imported));
    
    /* Simple computation to use all TLS variables */
    int sum = tls_used + tls_public + tls_weak + tls_hidden + 
              tls_protected + tls_static + tls_common;
    
    return sum > 0 ? 0 : 1;
}

#ifdef __cplusplus
}
#endif
