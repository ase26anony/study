/* Test for TLS emulation attribute copying - main file */
#include <stddef.h>

/* Prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(&var))

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

/* Test 4: Protected visibility TLS */
__attribute__((visibility("protected")))
__thread int tls_protected;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 5: Static TLS (non-public) with internal linkage */
static __thread int tls_static_internal = 7;
/* Tests: !TREE_PUBLIC, DECL_CONTEXT (function context) */

/* Test 6: External TLS declaration (defined in aux file) */
extern __thread int tls_external;
/* Tests: DECL_EXTERNAL */

/* Test 7: Common TLS (tentative definition) */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Test 8: DLL import style (simulated with weak external) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Use weak external as approximation */
__attribute__((weak)) extern __thread int tls_dllimport;
#endif
/* Tests: DECL_DLLIMPORT_P or DECL_WEAK + DECL_EXTERNAL */

/* Function to test TLS in different context */
static void test_function_context(void) {
    /* Test 9: TLS with function context */
    static __thread int tls_function_local = 99;
    /* Tests: DECL_CONTEXT (function), !TREE_PUBLIC */
    
    KEEP(tls_function_local);
    tls_function_local++;
}

/* Helper to ensure all variables are referenced */
static void reference_all_tls(void) {
    /* Take addresses to prevent optimization */
    int *ptrs[] = {
        &tls_public_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_static_internal,
        &tls_external,
        &tls_common,
        &tls_dllimport,
        NULL
    };
    
    /* Use asm to keep references without actual use */
    for (int i = 0; ptrs[i]; i++) {
        KEEP(*ptrs[i]);
    }
    
    /* Modify some values to ensure they're alive */
    tls_public_used++;
    tls_hidden += 2;
    tls_static_internal *= 3;
    
    /* Call function with local TLS */
    test_function_context();
}

/* Declaration from aux file */
void aux_function(void);

int main(void) {
    /* Reference all TLS variables */
    reference_all_tls();
    
    /* Call aux function which uses extern TLS */
    aux_function();
    
    /* Simple validation */
    if (tls_public_used > 0 && tls_hidden > 0) {
        return 0;
    }
    return 1;
}

/* Force inclusion in object file */
void __attribute__((constructor)) init_tls(void) {
    /* Ensure all TLS vars are initialized */
    tls_weak = 1;
    tls_protected = 2;
    tls_common = 3;
}
