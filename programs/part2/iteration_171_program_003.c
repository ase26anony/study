/* Test program for TLS emulation attribute copying coverage */
/* This tests the copy_decl_attributes function in tree-emutls.cc */

#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: TLS variable with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__attribute__((used))
__thread int tls_used_default = 42;

/* Test 2: TLS variable with hidden visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;

/* Test 3: Weak TLS variable */
/* Tests: DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 200;

/* Test 4: Common TLS variable (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common;

/* Test 5: External TLS declaration (defined in another file) */
/* Tests: DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 6: TLS variable with protected visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("protected")))
__thread int tls_protected = 300;

/* Test 7: Static TLS variable (non-public) */
/* Tests: TREE_PUBLIC (should be false), DECL_CONTEXT */
static __thread int tls_static = 400;

/* Function to test TLS variables in different context */
static void test_function_context(void) {
    /* Test 8: TLS variable with function context */
    /* Tests: DECL_CONTEXT (non-NULL) */
    static __thread int tls_function_local = 500;
    
    /* Use the variable to prevent optimization */
    tls_function_local++;
    KEEP_ALIVE(tls_function_local);
}

/* Test 9: TLS variable with internal visibility */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("internal")))
__thread int tls_internal = 600;

/* For C++ compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* Declaration for weak alias target */
__thread int tls_weak_alias_target = 700;

/* Function that uses all TLS variables */
void use_tls_variables(void) {
    /* Use each variable to ensure they're not optimized away */
    tls_used_default++;
    tls_hidden++;
    tls_weak++;
    tls_common = 1;
    tls_protected++;
    tls_static++;
    tls_internal++;
    
    /* Take addresses to ensure they're processed */
    KEEP_ALIVE(tls_used_default);
    KEEP_ALIVE(tls_hidden);
    KEEP_ALIVE(tls_weak);
    KEEP_ALIVE(tls_common);
    KEEP_ALIVE(tls_external);
    KEEP_ALIVE(tls_protected);
    KEEP_ALIVE(tls_static);
    KEEP_ALIVE(tls_internal);
    KEEP_ALIVE(tls_weak_alias_target);
    
    test_function_context();
}

#ifdef __cplusplus
}
#endif

int main(void) {
    /* Initialize common variable */
    tls_common = 50;
    
    /* Use all TLS variables */
    use_tls_variables();
    
    /* Test function context TLS */
    test_function_context();
    
    /* Simple validation */
    if (tls_used_default > 42 && tls_hidden > 100) {
        return 0;  /* Success */
    }
    
    return 1;
}
