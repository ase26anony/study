/* Test for TLS emulation attribute copying - Main file */
#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP(var) asm volatile("" : : "r"(&(var)))

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

/* Test 6: Static TLS with internal linkage */
static __thread int tls_static = 999;
/* Tests: TREE_PUBLIC (false), DECL_CONTEXT (non-NULL) */

/* Test 7: External declaration (defined in aux file) */
extern __thread int tls_external;
/* Tests: DECL_EXTERNAL */

/* Test 8: DLL import style (using weak external) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__attribute__((weak)) extern __thread int tls_dllimport;
#endif
/* Tests: DECL_DLLIMPORT_P or DECL_WEAK */

/* Function to use TLS variables in different scopes */
static void use_tls_variables(void) {
    /* Local TLS inside function - tests DECL_CONTEXT */
    static __thread int tls_local_func;
    
    /* Take addresses to prevent optimization */
    int *ptrs[] = {
        &tls_public_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_common,
        &tls_static,
        &tls_external,
        &tls_dllimport,
        &tls_local_func
    };
    
    /* Modify some variables */
    tls_public_used += 1;
    tls_hidden *= 2;
    tls_static -= 50;
    
    /* Force all variables to be considered used */
    for (size_t i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
        KEEP(*ptrs[i]);
    }
}

/* Function from aux file */
extern void aux_tls_operations(void);

int main(void) {
    /* Initialize some TLS variables */
    tls_common = 1234;
    tls_weak = 5678;
    
    /* Use TLS variables */
    use_tls_variables();
    
    /* Call aux function that uses external TLS */
    aux_tls_operations();
    
    /* Simple validation */
    if (tls_public_used != 43) return 1;
    if (tls_hidden != 200) return 2;
    if (tls_static != 949) return 3;
    
    return 0;
}

/* Force generation of TLS emulation structures */
#ifdef __cplusplus
extern "C" {
#endif

/* Additional TLS in C++ context if compiled as C++ */
#ifdef __cplusplus
namespace {
    __thread int tls_in_namespace = 777;
}
#endif

#ifdef __cplusplus
}
#endif
