/* Test for TLS emulation attribute copying coverage */
/* This file contains various TLS variables with different attributes */

#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Public TLS variable with default visibility and used attribute */
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

/* Test 5: Static TLS with internal linkage (non-public) */
static __thread int tls_static_internal;
/* Tests: !TREE_PUBLIC, DECL_CONTEXT (function context if in function) */

/* Test 6: Common TLS variable (tentative definition) */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Test 7: DLL import simulation (using weak external) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with weak external on non-Windows */
__attribute__((weak))
extern __thread int tls_dllimport;
#endif
/* Tests: DECL_DLLIMPORT_P or DECL_WEAK + DECL_EXTERNAL */

/* Function-scoped TLS to test DECL_CONTEXT */
void test_function_context(void) {
    /* TLS inside function scope */
    static __thread int tls_function_scope = 99;
    /* Tests: DECL_CONTEXT (function context) */
    
    KEEP_ALIVE(tls_function_scope);
    tls_function_scope++;
}

/* External function from aux file */
extern void use_external_tls(void);

int main(void) {
    /* Use all TLS variables to ensure they're processed */
    
    /* Test 1 */
    KEEP_ALIVE(tls_public_used);
    tls_public_used++;
    
    /* Test 2 */
    KEEP_ALIVE(tls_weak);
    tls_weak = 1;
    
    /* Test 3 */
    KEEP_ALIVE(tls_hidden);
    tls_hidden++;
    
    /* Test 4 - will be defined in aux file */
    KEEP_ALIVE(tls_external_protected);
    
    /* Test 5 */
    KEEP_ALIVE(tls_static_internal);
    tls_static_internal = 5;
    
    /* Test 6 */
    KEEP_ALIVE(tls_common);
    tls_common = 10;
    
    /* Test 7 */
    KEEP_ALIVE(tls_dllimport);
    
    /* Test function context */
    test_function_context();
    
    /* Use external TLS variables */
    use_external_tls();
    
    return 0;
}

/* Force visibility attributes to be applied */
#ifdef __GNUC__
__attribute__((visibility("default")))
#endif
void dummy_export(void) {}
