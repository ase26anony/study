/* Test for TLS emulation attribute copying - Main file */
#include <stddef.h>

/* Prevent optimization from removing unused TLS variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Public TLS with default visibility and used attribute */
__attribute__((used)) __thread int tls_public_used = 42;
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */

/* Test 2: Weak TLS variable */
__attribute__((weak)) __thread int tls_weak;
/* Tests: DECL_WEAK */

/* Test 3: Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 100;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 4: Protected visibility TLS */
__attribute__((visibility("protected"))) __thread int tls_protected;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 5: Common TLS (tentative definition) */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Test 6: External TLS declaration (defined in aux file) */
extern __thread int tls_external;

/* Test 7: Static TLS with internal linkage */
static __thread int tls_static = 7;

/* Test 8: TLS inside function scope for DECL_CONTEXT */
static void test_function_scope(void) {
    /* This creates a DECL_CONTEXT */
    static __thread int tls_in_function = 99;
    KEEP_ALIVE(tls_in_function);
}

/* Test 9: DLL import style attribute (if supported) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
/* Simulate DLL import with visibility and weak */
__attribute__((weak, visibility("default"))) __thread int tls_dllimport;
#else
__thread int tls_dllimport;
#endif
/* Tests: DECL_DLLIMPORT_P or DECL_WEAK + DECL_VISIBILITY */

/* Test 10: Complex combination - weak, hidden, used */
__attribute__((weak, visibility("hidden"), used)) 
__thread int tls_complex;

/* Function from aux file */
extern void use_tls_variables(void);

int main(void) {
    /* Ensure all TLS variables are referenced to prevent elimination */
    
    /* Test 1: Public used */
    tls_public_used += 1;
    KEEP_ALIVE(tls_public_used);
    
    /* Test 2: Weak */
    tls_weak = 10;
    KEEP_ALIVE(tls_weak);
    
    /* Test 3: Hidden */
    tls_hidden *= 2;
    KEEP_ALIVE(tls_hidden);
    
    /* Test 4: Protected */
    tls_protected = 50;
    KEEP_ALIVE(tls_protected);
    
    /* Test 5: Common */
    tls_common = 123;
    KEEP_ALIVE(tls_common);
    
    /* Test 6: External */
    KEEP_ALIVE(tls_external);
    
    /* Test 7: Static */
    tls_static++;
    KEEP_ALIVE(tls_static);
    
    /* Test 8: Function scope */
    test_function_scope();
    
    /* Test 9: DLL import style */
    KEEP_ALIVE(tls_dllimport);
    
    /* Test 10: Complex */
    tls_complex = 999;
    KEEP_ALIVE(tls_complex);
    
    /* Use variables from aux file */
    use_tls_variables();
    
    return 0;
}
