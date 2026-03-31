/* Test program for TLS emulation attribute copying coverage */
/* This tests the copy_decl_attributes function in tree-emutls.cc */

#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: TLS with used attribute (tests DECL_PRESERVE_P) */
__thread int tls_used __attribute__((used)) = 42;

/* Test 2: Public TLS with default visibility (tests TREE_PUBLIC, DECL_VISIBILITY) */
__thread int tls_public = 100;

/* Test 3: Weak TLS variable (tests DECL_WEAK) */
__thread int tls_weak __attribute__((weak)) = 200;

/* Test 4: TLS with hidden visibility (tests DECL_VISIBILITY_SPECIFIED) */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* Test 5: TLS with protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* Test 6: External TLS declaration (tests DECL_EXTERNAL) */
extern __thread int tls_external;

/* Test 7: Common TLS (tests DECL_COMMON) - tentative definition */
__thread int tls_common;

/* Test 8: Static TLS with internal linkage (contrast for TREE_PUBLIC) */
static __thread int tls_static = 500;

/* Test 9: TLS in function scope (tests DECL_CONTEXT) */
static void test_function_scope(void) {
    __thread int tls_local = 600;
    KEEP_ALIVE(tls_local);
}

/* Test 10: DLL import style attribute if supported */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
/* Simulate DLL import with visibility */
__thread int tls_dllimport __attribute__((visibility("default"), weak));
#endif

/* Function that uses all TLS variables to ensure they're processed */
void use_all_tls_vars(void) {
    /* Take addresses to prevent optimization */
    int *ptrs[] = {
        &tls_used,
        &tls_public,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_external,
        &tls_common,
        &tls_static,
        #ifdef _WIN32
        &tls_dllimport,
        #endif
        NULL
    };
    
    /* Modify some values */
    tls_used += 1;
    tls_public *= 2;
    tls_weak -= 1;
    tls_hidden = tls_hidden * 3;
    tls_protected = tls_protected / 2;
    tls_common = 999;
    tls_static++;
    
    /* Ensure all are kept alive */
    for (int i = 0; ptrs[i] != NULL; i++) {
        KEEP_ALIVE(*ptrs[i]);
    }
}

/* Main function */
int main(void) {
    /* Initialize common TLS */
    tls_common = 777;
    
    /* Use function scope TLS */
    test_function_scope();
    
    /* Use all TLS variables */
    use_all_tls_vars();
    
    /* Reference external TLS */
    tls_external = 888;
    
    /* Return something based on TLS values */
    return (tls_used + tls_public + tls_weak + tls_hidden + 
            tls_protected + tls_external + tls_common + tls_static) % 256;
}
