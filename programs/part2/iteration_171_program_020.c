/* Test program for TLS emulation attribute copying coverage */
/* This tests the copy_decl_attributes function in tree-emutls.cc */

#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: TLS variable with used attribute and default visibility */
/* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__attribute__((used)) __thread int tls_used_public = 42;

/* Test 2: TLS variable with hidden visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 100;

/* Test 3: Weak TLS variable */
/* Tests: DECL_WEAK */
__attribute__((weak)) __thread int tls_weak;

/* Test 4: TLS variable in function scope (non-NULL DECL_CONTEXT) */
/* Tests: DECL_CONTEXT */
static void test_function_scope(void) {
    static __thread int tls_function_local = 999;
    KEEP_ALIVE(tls_function_local);
}

/* Test 5: External TLS declaration (will be defined in aux file) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 6: Common TLS variable (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common;

/* Test 7: Protected visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("protected"))) __thread int tls_protected = 200;

/* Test 8: Internal visibility TLS */
#ifdef __GNUC__
__attribute__((visibility("internal"))) __thread int tls_internal = 300;
#endif

/* Forward declaration for function in aux file */
void aux_function(void);

int main(void) {
    /* Force usage of all TLS variables to prevent optimization */
    
    /* Test 1: Used public TLS */
    tls_used_public += 1;
    KEEP_ALIVE(tls_used_public);
    
    /* Test 2: Hidden TLS */
    tls_hidden = tls_used_public * 2;
    KEEP_ALIVE(tls_hidden);
    
    /* Test 3: Weak TLS */
    if (&tls_weak != NULL) {
        tls_weak = 123;
    }
    KEEP_ALIVE(tls_weak);
    
    /* Test 4: Function scope TLS */
    test_function_scope();
    
    /* Test 5: External TLS */
    tls_external = 456;
    KEEP_ALIVE(tls_external);
    
    /* Test 6: Common TLS */
    tls_common = 789;
    KEEP_ALIVE(tls_common);
    
    /* Test 7: Protected TLS */
    tls_protected = tls_used_public + tls_hidden;
    KEEP_ALIVE(tls_protected);
    
    /* Test 8: Internal TLS */
#ifdef __GNUC__
    tls_internal = tls_protected * 2;
    KEEP_ALIVE(tls_internal);
#endif
    
    /* Call function from aux file that uses TLS */
    aux_function();
    
    return 0;
}

/* Additional test: DLL import simulation for MinGW/Cygwin */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#endif

/* Test with C++ namespace for DECL_CONTEXT */
#ifdef __cplusplus
namespace test_namespace {
    __thread int tls_in_namespace = 555;
}
#endif
