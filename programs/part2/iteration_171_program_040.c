/* Test for TLS emulation attribute copying - Main file */
#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Public TLS with default visibility and used attribute */
__attribute__((used))
__thread int tls_public_used = 42;
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */

/* Test 2: Weak TLS variable */
__attribute__((weak))
__thread int tls_weak;
/* Tests: DECL_WEAK, DECL_COMMON (tentative definition) */

/* Test 3: Hidden visibility TLS */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 4: Protected visibility with external linkage */
__attribute__((visibility("protected")))
extern __thread int tls_external_protected;
/* Tests: DECL_EXTERNAL, DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 5: Internal visibility */
__attribute__((visibility("internal")))
__thread int tls_internal = 200;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 6: Static TLS (non-public) inside a function context */
static void test_context(void) {
    /* This creates a DECL_CONTEXT */
    static __thread int tls_static_context = 300;
    KEEP_ALIVE(tls_static_context);
}
/* Tests: DECL_CONTEXT, !TREE_PUBLIC */

/* Test 7: Common (tentative definition) with default visibility */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Forward declaration for function in auxiliary file */
void use_external_tls(void);

int main(void) {
    /* Use all TLS variables to ensure they're not optimized away */
    
    /* Test 1: Public used */
    tls_public_used += 1;
    KEEP_ALIVE(tls_public_used);
    
    /* Test 2: Weak */
    tls_weak = 50;
    KEEP_ALIVE(tls_weak);
    
    /* Test 3: Hidden */
    tls_hidden *= 2;
    KEEP_ALIVE(tls_hidden);
    
    /* Test 4: External protected (defined in aux file) */
    KEEP_ALIVE(tls_external_protected);
    
    /* Test 5: Internal */
    tls_internal = tls_internal + 10;
    KEEP_ALIVE(tls_internal);
    
    /* Test 6: Static with context */
    test_context();
    
    /* Test 7: Common */
    tls_common = 99;
    KEEP_ALIVE(tls_common);
    
    /* Use external TLS from auxiliary file */
    use_external_tls();
    
    return 0;
}

/* DLL import simulation for MinGW/Cygwin targets */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
/* Tests: DECL_DLLIMPORT_P */
#else
/* On non-Windows, simulate with weak attribute */
__attribute__((weak)) __thread int tls_dllimport;
#endif

/* Function that takes address of DLL import variable */
void use_dllimport_var(void) {
    KEEP_ALIVE(tls_dllimport);
}
