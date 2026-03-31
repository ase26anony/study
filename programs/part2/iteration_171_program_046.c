/* Test for TLS emulation attribute copying - main file */
/* This tests the copy_decl_attributes function in tree-emutls.cc */

#ifdef __cplusplus
extern "C" {
#endif

/* Force TLS emulation if not already enabled */
#ifndef __EMUTLS__
#define __EMUTLS__ 1
#endif

/* Test 1: TLS variable with used attribute - tests DECL_PRESERVE_P */
__thread int tls_used __attribute__((used)) = 42;

/* Test 2: Public TLS variable - tests TREE_PUBLIC */
__thread int tls_public = 100;

/* Test 3: Weak TLS variable - tests DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 200;

/* Test 4: Hidden visibility - tests DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* Test 5: Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* Test 6: Common linkage (tentative definition) - tests DECL_COMMON */
__thread int tls_common;

/* Test 7: External declaration (defined in aux file) - tests DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 8: DLL import style attribute if supported */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Test 9: Static TLS with context (inside function) */
static void test_context(void) {
    /* Local static TLS - will have DECL_CONTEXT set to the function */
    static __thread int tls_local_static = 500;
    
    /* Use it to prevent optimization */
    asm volatile("" : : "r"(&tls_local_static));
}

/* Test 10: TLS with default visibility explicitly specified */
__thread int tls_default_vis __attribute__((visibility("default"))) = 600;

/* Function that uses all TLS variables to ensure they're not optimized away */
void use_all_tls(void) {
    /* Take addresses to force TLS processing */
    int *ptrs[] = {
        &tls_used,
        &tls_public,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_common,
        &tls_external,
#ifdef __GNUC__
        &tls_dllimport,
#endif
        &tls_default_vis
    };
    
    /* Use asm to prevent optimization */
    for (int i = 0; i < (int)(sizeof(ptrs)/sizeof(ptrs[0])); i++) {
        asm volatile("" : : "r"(ptrs[i]));
    }
    
    /* Modify some values */
    tls_used += 1;
    tls_public += 2;
    tls_common = 999;
    
    /* Call context test */
    test_context();
}

/* Main function */
int main(void) {
    /* Initialize common TLS */
    tls_common = 777;
    
    /* Use all TLS variables */
    use_all_tls();
    
    /* Simple validation */
    if (tls_used != 43) return 1;
    if (tls_public != 102) return 2;
    if (tls_common != 999) return 3;
    
    return 0;
}

#ifdef __cplusplus
}
#endif
