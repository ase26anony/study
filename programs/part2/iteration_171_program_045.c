/* Test program for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with used attribute (tests DECL_PRESERVE_P) */
__thread int tls_used_var __attribute__((used)) = 42;

/* Test 2: Public TLS variable (tests TREE_PUBLIC) */
__thread int tls_public_var = 100;

/* Test 3: Weak TLS variable (tests DECL_WEAK) */
__thread int tls_weak_var __attribute__((weak)) = 200;

/* Test 4: Hidden visibility (tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED) */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 300;

/* Test 5: Protected visibility */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 400;

/* Test 6: Internal visibility */
#ifdef __GNUC__
__thread int tls_internal_var __attribute__((visibility("internal"))) = 500;
#else
__thread int tls_internal_var = 500;
#endif

/* Test 7: DLL import attribute (tests DECL_DLLIMPORT_P) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_dllimport_var;
#else
/* On non-Windows, simulate with external declaration */
extern __thread int tls_dllimport_var;
#endif

/* Test 8: Common linkage (tests DECL_COMMON) */
/* Tentative definition - should get common linkage */
__thread int tls_common_var;

/* Test 9: External declaration (tests DECL_EXTERNAL) */
extern __thread int tls_external_var;

/* Test 10: Static TLS (non-public, for contrast) */
static __thread int tls_static_var = 600;

/* Function to test TLS in function scope (tests DECL_CONTEXT) */
static void test_function_scope(void) {
    /* TLS variable with function scope */
    static __thread int tls_function_scope = 700;
    
    /* Take address to prevent optimization */
    volatile int *ptr = &tls_function_scope;
    (void)ptr;
}

/* External function from auxiliary file */
void aux_function(void);

int main(void) {
    /* Test 1: Used variable - read and modify */
    tls_used_var += 1;
    
    /* Test 2: Public variable - take address */
    volatile int *public_ptr = &tls_public_var;
    (void)public_ptr;
    
    /* Test 3: Weak variable - use in asm to ensure preservation */
    asm volatile("" : : "r"(&tls_weak_var));
    
    /* Test 4: Hidden visibility - modify */
    tls_hidden_var = tls_hidden_var * 2;
    
    /* Test 5: Protected visibility - take address */
    volatile int *protected_ptr = &tls_protected_var;
    (void)protected_ptr;
    
    /* Test 6: Internal visibility - use */
    tls_internal_var = 501;
    
    /* Test 7: DLL import - reference (will be defined in aux file) */
    /* tls_dllimport_var is extern, will be resolved from aux file */
    
    /* Test 8: Common linkage - initialize if not already */
    if (tls_common_var == 0) {
        tls_common_var = 800;
    }
    
    /* Test 9: External variable - reference */
    /* tls_external_var is defined in aux file */
    
    /* Test 10: Static TLS - use */
    tls_static_var++;
    
    /* Test function scope TLS */
    test_function_scope();
    
    /* Call auxiliary function */
    aux_function();
    
    /* Prevent dead code elimination for all variables */
    asm volatile("" : : 
        "r"(&tls_used_var),
        "r"(&tls_public_var),
        "r"(&tls_weak_var),
        "r"(&tls_hidden_var),
        "r"(&tls_protected_var),
        "r"(&tls_internal_var)
    );
    
    return 0;
}

#ifdef __cplusplus
}
#endif
