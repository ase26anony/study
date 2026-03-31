/* Test for TLS emulation attribute copying coverage */
/* This tests the copy_decl_attributes function in tree-emutls.cc */

#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP(var) asm volatile("" : : "r"(&(var)))

/* Test 1: TLS with used attribute - tests DECL_PRESERVE_P */
__thread int tls_used __attribute__((used)) = 42;

/* Test 2: Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int tls_public = 100;

/* Test 3: Weak TLS variable - tests DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 200;

/* Test 4: TLS with hidden visibility - tests DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* Test 5: External TLS declaration (defined in aux file) - tests DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 6: Common TLS (tentative definition) - tests DECL_COMMON */
__thread int tls_common;

/* Test 7: DLL import style (using weak alias) - tests DECL_DLLIMPORT_P */
/* We'll simulate this with weak alias to another variable */
__thread int tls_dllimport_target = 500;
extern __thread int tls_dllimport __attribute__((weak, alias("tls_dllimport_target")));

/* Test 8: Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 600;

/* Test 9: Static TLS (non-public) for contrast */
static __thread int tls_static = 700;

/* Test 10: TLS in function scope - tests DECL_CONTEXT */
static void test_function_scope(void) {
    __thread int tls_local = 800;
    KEEP(tls_local);
}

/* Helper function to use all TLS variables */
static void use_all_tls(void) {
    /* Take addresses to ensure they're not optimized away */
    int *ptrs[] = {
        &tls_used,
        &tls_public,
        &tls_weak,
        &tls_hidden,
        &tls_external,
        &tls_common,
        &tls_dllimport,
        &tls_protected,
        &tls_static,
        NULL
    };
    
    /* Read-modify-write on some variables */
    tls_used += 1;
    tls_public *= 2;
    tls_weak -= 1;
    tls_hidden = tls_hidden + 100;
    tls_common = 999;
    tls_protected = tls_protected / 2;
    tls_static = 1234;
    
    /* Ensure all are marked as used */
    for (int i = 0; ptrs[i] != NULL; i++) {
        KEEP(*ptrs[i]);
    }
}

/* Function that returns address of TLS for external use */
int* get_tls_public_addr(void) {
    return &tls_public;
}

int* get_tls_weak_addr(void) {
    return &tls_weak;
}

/* Main function */
int main(void) {
    /* Initialize common TLS */
    tls_common = 888;
    
    /* Use function scope TLS */
    test_function_scope();
    
    /* Use all TLS variables */
    use_all_tls();
    
    /* External TLS should be defined in aux file */
    tls_external = 111;
    
    /* Simple validation */
    if (tls_used != 43) return 1;
    if (tls_public != 200) return 2;
    if (tls_common != 999) return 3;
    
    return 0;
}
