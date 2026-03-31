/* Test program for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Basic TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__attribute__((used))
__thread int tls_used_public = 42;

/* Test 2: Static TLS with hidden visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
static __thread int tls_static_hidden = 100;

/* Test 3: External TLS declaration (will be defined in aux file) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 4: Common TLS (tentative definition) */
/* Tests: DECL_COMMON, TREE_PUBLIC */
__thread int tls_common;

/* Test 5: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__attribute__((weak))
__thread int tls_weak = 200;

/* Test 6: Protected visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, TREE_PUBLIC */
__attribute__((visibility("protected")))
__thread int tls_protected = 300;

/* Test 7: Internal visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("internal")))
static __thread int tls_internal = 400;

/* Function to create DECL_CONTEXT for some variables */
static void create_context(void) {
    /* Test 8: TLS inside function scope */
    /* Tests: DECL_CONTEXT */
    static __thread int tls_function_scope = 500;
    
    /* Use it to prevent optimization */
    tls_function_scope++;
    KEEP_ALIVE(tls_function_scope);
}

/* Test 9: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
/* Simulate with visibility and weak on non-Windows */
#define DLL_IMPORT __attribute__((weak, visibility("default")))
#endif

/* Tests: DECL_DLLIMPORT_P or equivalent */
DLL_IMPORT extern __thread int tls_dll_import;

/* Helper function from aux file */
extern void use_tls_variables(void);

int main(void) {
    int result = 0;
    
    /* Ensure all TLS variables are referenced */
    
    /* Test 1: Used public TLS */
    tls_used_public += 1;
    result += tls_used_public;
    
    /* Test 2: Static hidden TLS */
    tls_static_hidden += 2;
    result += tls_static_hidden;
    KEEP_ALIVE(tls_static_hidden);
    
    /* Test 3: External TLS (defined in aux) */
    result += tls_external;
    KEEP_ALIVE(tls_external);
    
    /* Test 4: Common TLS */
    tls_common = 50;
    result += tls_common;
    
    /* Test 5: Weak TLS */
    if (&tls_weak != NULL) {
        tls_weak += 3;
        result += tls_weak;
    }
    
    /* Test 6: Protected TLS */
    tls_protected += 4;
    result += tls_protected;
    
    /* Test 7: Internal TLS */
    tls_internal += 5;
    result += tls_internal;
    KEEP_ALIVE(tls_internal);
    
    /* Test 8: Function scope TLS */
    create_context();
    
    /* Test 9: DLL import TLS */
#ifdef tls_dll_import_available
    result += tls_dll_import;
    KEEP_ALIVE(tls_dll_import);
#endif
    
    /* Take addresses to ensure variables exist */
    void* addresses[] = {
        &tls_used_public,
        &tls_static_hidden,
        &tls_external,
        &tls_common,
        &tls_weak,
        &tls_protected,
        &tls_internal,
    };
    
    /* Use addresses to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        KEEP_ALIVE(addresses[i]);
    }
    
    /* Call function from aux file */
    use_tls_variables();
    
    return result > 0 ? 0 : 1;
}
