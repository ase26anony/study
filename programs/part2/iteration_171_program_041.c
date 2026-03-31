/* Test program for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

/* Include for attribute compatibility */
#include <stddef.h>

/* Test 1: Basic TLS with used attribute - tests DECL_PRESERVE_P */
__thread int tls_used_var __attribute__((used)) = 42;

/* Test 2: Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int tls_public_var __attribute__((visibility("default"))) = 100;

/* Test 3: Weak TLS variable - tests DECL_WEAK */
__thread int tls_weak_var __attribute__((weak)) = 200;

/* Test 4: Hidden visibility - tests DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 300;

/* Test 5: Protected visibility */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 400;

/* Test 6: Common linkage (tentative definition) - tests DECL_COMMON */
__thread int tls_common_var;

/* Test 7: External declaration (defined in aux file) - tests DECL_EXTERNAL */
extern __thread int tls_external_var;

/* Test 8: Static TLS with context - tests DECL_CONTEXT */
static void test_function(void) {
    /* Local static TLS - will have function as DECL_CONTEXT */
    static __thread int tls_local_static = 500;
    
    /* Take address to prevent optimization */
    volatile int *ptr = &tls_local_static;
    (void)ptr;
}

/* Test 9: DLL import simulation (for MinGW/Cygwin) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_dllimport_var;
#else
/* On non-Windows, use weak external as approximation */
extern __thread int tls_dllimport_var __attribute__((weak));
#endif

/* Test 10: Multiple combined attributes */
__thread int tls_combined __attribute__((used, visibility("internal"), weak));

/* Forward declaration for aux function */
void aux_function(void);

int main(void) {
    /* Force usage of all TLS variables to prevent dead code elimination */
    
    /* Test 1: Used variable */
    tls_used_var += 1;
    asm volatile("" : : "r"(&tls_used_var));
    
    /* Test 2: Public variable */
    tls_public_var *= 2;
    asm volatile("" : : "r"(&tls_public_var));
    
    /* Test 3: Weak variable */
    int *weak_ptr = &tls_weak_var;
    (void)weak_ptr;
    
    /* Test 4: Hidden variable */
    tls_hidden_var -= 10;
    asm volatile("" : : "r"(&tls_hidden_var));
    
    /* Test 5: Protected variable */
    volatile int *prot_ptr = &tls_protected_var;
    (void)prot_ptr;
    
    /* Test 6: Common variable */
    tls_common_var = 600;
    
    /* Test 7: External variable (defined in aux) */
    tls_external_var = 700;
    
    /* Test 8: Call function with local static TLS */
    test_function();
    
    /* Test 9: DLL import style variable */
#ifdef __GNUC__
    asm volatile("" : : "r"(&tls_dllimport_var));
#endif
    
    /* Test 10: Combined attributes */
    tls_combined = 800;
    
    /* Call aux function to use externals */
    aux_function();
    
    /* Simple validation */
    int sum = tls_used_var + tls_public_var + tls_hidden_var + 
              tls_protected_var + tls_common_var;
    
    return (sum > 0) ? 0 : 1;
}

#ifdef __cplusplus
}
#endif
