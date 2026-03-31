/* Test for TLS emulation attribute copying - Main file */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#include <stddef.h>

/* Prevent optimization from removing unused TLS variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Public TLS variable with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */
__thread int tls_public_used __attribute__((used)) = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__thread int tls_weak __attribute__((weak)) = 100;

/* Test 3: Hidden visibility TLS variable */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* Test 4: Protected visibility TLS variable */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_protected __attribute__((visibility("protected"))) = 300;

/* Test 5: Common TLS variable (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common;

/* Test 6: Static TLS variable (non-public, has DECL_CONTEXT) */
/* Tests: DECL_CONTEXT (function scope) */
static void test_function_scope(void) {
    static __thread int tls_static_func = 500;
    KEEP_ALIVE(tls_static_func);
}

/* Test 7: External TLS declaration (defined in emutls_aux.c) */
/* Tests: DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 8: DLL import style attribute (simulated with weak) */
/* Tests: DECL_DLLIMPORT_P (when available) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* On non-Windows, use weak import as approximation */
__thread int tls_weak_import __attribute__((weak));
#endif

/* Test 9: Internal visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* Function that uses all TLS variables to ensure they're not optimized away */
void use_all_tls_vars(void) {
    /* Take addresses to force TLS emulation structure creation */
    int *ptrs[] = {
        &tls_public_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_common,
        &tls_external,
#ifdef _WIN32
        &tls_dllimport,
#else
        &tls_weak_import,
#endif
        &tls_internal,
        NULL
    };
    
    /* Modify some values */
    tls_public_used += 1;
    tls_hidden *= 2;
    tls_protected -= 50;
    tls_common = 999;
    
    /* Call function from other file that uses external TLS */
    use_external_tls();
    
    /* Test function scope TLS */
    test_function_scope();
}

/* Main function */
int main(void) {
    /* Initialize coverage profiling */
    __gcov_flush();
    
    /* Use all TLS variables */
    use_all_tls_vars();
    
    /* Keep all variables alive for coverage */
    KEEP_ALIVE(tls_public_used);
    KEEP_ALIVE(tls_weak);
    KEEP_ALIVE(tls_hidden);
    KEEP_ALIVE(tls_protected);
    KEEP_ALIVE(tls_common);
    KEEP_ALIVE(tls_external);
#ifdef _WIN32
    KEEP_ALIVE(tls_dllimport);
#else
    KEEP_ALIVE(tls_weak_import);
#endif
    KEEP_ALIVE(tls_internal);
    
    return 0;
}

/* Force inclusion in DECL_CONTEXT by being in file scope */
static struct {
    __thread int tls_in_struct;
} context_struct;

__thread int tls_after_struct = 700;
