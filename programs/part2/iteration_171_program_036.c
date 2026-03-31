/* Test for TLS emulation attribute copying - Main file */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Public TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */
__attribute__((used))
__thread int tls_public_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__attribute__((weak))
__thread int tls_weak_var = 100;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 200;

/* Test 4: Protected visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("protected")))
__thread int tls_protected = 300;

/* Test 5: Static TLS (non-public) with internal linkage */
/* Tests: !TREE_PUBLIC, DECL_CONTEXT (implicitly from file scope) */
static __thread int tls_static_internal = 400;

/* Test 6: Common TLS (tentative definition) */
/* Tests: DECL_COMMON, TREE_PUBLIC */
__thread int tls_common;

/* Test 7: External TLS declaration (defined in aux file) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external_defined;

/* Test 8: DLL import style attribute (simulated with weak) */
/* Tests: DECL_DLLIMPORT_P (on appropriate targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* On non-Windows, use weak import as approximation */
__attribute__((weak)) __thread int tls_dllimport;
#endif

/* Function to use TLS variables and prevent optimization */
static void use_tls_variables(void) {
    /* Take addresses to ensure variables are referenced */
    volatile int *ptr;
    
    ptr = &tls_public_used;
    tls_public_used += 1;
    
    ptr = &tls_weak_var;
    tls_weak_var += 2;
    
    ptr = &tls_hidden;
    tls_hidden += 3;
    
    ptr = &tls_protected;
    tls_protected += 4;
    
    ptr = &tls_static_internal;
    tls_static_internal += 5;
    
    ptr = &tls_common;
    tls_common = 600;
    
    ptr = &tls_external_defined;
    /* Will be defined in aux file */
    
    ptr = &tls_dllimport;
    
    /* Use inline asm to prevent dead code elimination */
    __asm__ volatile("" : : "r"(ptr) : "memory");
}

/* Nested function scope TLS to test DECL_CONTEXT */
void outer_function(void) {
    /* Test 9: TLS in function scope */
    /* Tests: DECL_CONTEXT (function context) */
    static __thread int tls_in_function = 700;
    
    tls_in_function++;
    volatile int *ptr = &tls_in_function;
    __asm__ volatile("" : : "r"(ptr) : "memory");
}

/* Main function */
int main(void) {
    /* Initialize coverage profiling */
    int i;
    
    /* Use all TLS variables multiple times */
    for (i = 0; i < 10; i++) {
        use_tls_variables();
        outer_function();
    }
    
    /* Reference variables via inline asm to ensure they're not optimized out */
    __asm__ volatile(
        "/* TLS variable references */"
        :
        : "r"(tls_public_used), "r"(tls_weak_var), "r"(tls_hidden),
          "r"(tls_protected), "r"(tls_static_internal), "r"(tls_common)
        : "memory"
    );
    
    return 0;
}

#ifdef __cplusplus
}
#endif
