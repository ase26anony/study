/* Test program for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__thread int tls_used __attribute__((used)) = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__thread int tls_weak __attribute__((weak)) = 100;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, TREE_PUBLIC */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* Test 4: Protected visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, TREE_PUBLIC */
__thread int tls_protected __attribute__((visibility("protected"))) = 300;

/* Test 5: Internal visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, TREE_PUBLIC */
#ifdef __GNUC__
__thread int tls_internal __attribute__((visibility("internal"))) = 400;
#else
/* Fallback for compilers without internal visibility */
__thread int tls_internal = 400;
#endif

/* Test 6: Common TLS (tentative definition) - tests DECL_COMMON */
__thread int tls_common;

/* Test 7: Static TLS with context - tests DECL_CONTEXT (non-NULL) */
static void func_with_tls(void) {
    /* TLS inside function scope gives it a DECL_CONTEXT */
    static __thread int tls_in_function = 700;
    
    /* Use it to prevent optimization */
    tls_in_function++;
    asm volatile("" : : "r"(&tls_in_function));
}

/* Test 8: External TLS declaration (defined in aux file) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 9: DLL import style attribute if supported */
/* Tests: DECL_DLLIMPORT_P */
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
/* Simulate DLL import with visibility and weak */
__thread int tls_dllimport __attribute__((weak, visibility("default")));
#else
__thread int tls_dllimport;
#endif

/* Helper function to use all TLS variables */
void use_all_tls(void) {
    /* Take addresses to ensure variables are processed */
    int *ptrs[] = {
        &tls_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_internal,
        &tls_common,
        &tls_external,
        &tls_dllimport
    };
    
    /* Read-modify-write operations */
    tls_used += 1;
    tls_weak += 2;
    tls_hidden += 3;
    tls_protected += 4;
    tls_internal += 5;
    tls_common = 600;
    
    /* Use asm to prevent optimization */
    for (size_t i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
        asm volatile("" : : "r"(ptrs[i]));
    }
    
    /* Call function with static TLS */
    func_with_tls();
}

/* Main function */
int main(void) {
    /* Initialize coverage */
    int dummy = 0;
    asm volatile("" : "+r"(dummy));
    
    /* Use all TLS variables */
    use_all_tls();
    
    /* Additional operations to ensure coverage */
    
    /* Test weak alias access */
    if (&tls_weak != NULL) {
        tls_weak = 150;
    }
    
    /* Test external variable (defined in aux file) */
    tls_external = 999;
    
    /* Test DLL import style */
    tls_dllimport = 1111;
    
    /* Ensure all are truly used */
    volatile int sum = 
        tls_used + tls_weak + tls_hidden + 
        tls_protected + tls_internal + tls_common;
    
    (void)sum; /* Suppress unused warning */
    
    return 0;
}

#ifdef __cplusplus
}
#endif
