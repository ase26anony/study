/* Test for TLS emulation attribute copying - Main file */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Basic TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__attribute__((used))
__thread int tls_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__attribute__((weak))
__thread int tls_weak;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, TREE_PUBLIC */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;

/* Test 4: Protected visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, TREE_PUBLIC */
__attribute__((visibility("protected")))
__thread int tls_protected;

/* Test 5: Common TLS (tentative definition) */
/* Tests: DECL_COMMON, TREE_PUBLIC */
__thread int tls_common;

/* Test 6: Static TLS (non-public) for contrast */
/* Tests: !TREE_PUBLIC, DECL_CONTEXT (file scope) */
static __thread int tls_static = 99;

/* Test 7: External declaration (defined in aux file) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 8: DLL import style (simulated with weak) */
/* Tests: DECL_DLLIMPORT_P (if supported), DECL_WEAK */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with weak attribute */
__attribute__((weak))
__thread int tls_dllimport;
#endif

/* Function with local TLS to test DECL_CONTEXT from function scope */
static void test_local_tls(void) {
    /* Test 9: TLS inside function scope */
    /* Tests: DECL_CONTEXT (function scope), !TREE_PUBLIC */
    static __thread int tls_local_func = 123;
    KEEP(tls_local_func);
}

/* Forward declaration for function in aux file */
void aux_function(void);

int main(void) {
    /* Ensure all TLS variables are referenced to prevent elimination */
    
    /* Test 1: Used TLS */
    tls_used += 1;
    KEEP(tls_used);
    
    /* Test 2: Weak TLS */
    tls_weak = 10;
    KEEP(tls_weak);
    
    /* Test 3: Hidden TLS */
    tls_hidden *= 2;
    KEEP(tls_hidden);
    
    /* Test 4: Protected TLS */
    tls_protected = 50;
    KEEP(tls_protected);
    
    /* Test 5: Common TLS */
    tls_common = 75;
    KEEP(tls_common);
    
    /* Test 6: Static TLS */
    tls_static++;
    KEEP(tls_static);
    
    /* Test 7: External TLS (defined in aux) */
    KEEP(tls_external);
    
    /* Test 8: DLL import style */
    tls_dllimport = 88;
    KEEP(tls_dllimport);
    
    /* Test local function TLS */
    test_local_tls();
    
    /* Call aux function to reference its TLS variables */
    aux_function();
    
    /* Take addresses to force TLS emulation structure generation */
    void* addresses[] = {
        &tls_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_common,
        &tls_static,
        &tls_external,
        &tls_dllimport
    };
    
    KEEP(addresses);
    
    return 0;
}

/* C++ compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* Test 10: TLS in namespace-like context (C++ would have DECL_CONTEXT) */
/* In C, we simulate with static inside a struct-like context */
struct container {
    static __thread int tls_in_struct;  /* C++ would set DECL_CONTEXT */
};

#ifdef __cplusplus
}
#endif
