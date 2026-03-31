/* Test for TLS emulation attribute copying - Main file */
#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

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

/* Test 5: Static TLS (non-public) inside a function to give it context */
static void func_with_tls(void) {
    static __thread int tls_in_func = 7;
    /* Tests: DECL_CONTEXT (non-NULL), !TREE_PUBLIC */
    KEEP_ALIVE(tls_in_func);
}

/* Test 6: Common TLS (tentative definition) */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Test 7: External TLS declaration (defined in aux file) */
extern __thread int tls_external;
/* Tests: DECL_EXTERNAL */

/* Test 8: DLL import style attribute (for MinGW/Cygwin targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
__attribute__((dllimport)) __thread int tls_dllimport;
#endif
/* Tests: DECL_DLLIMPORT_P */

/* Test 9: Internal visibility */
__attribute__((visibility("internal"))) __thread int tls_internal = 999;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 10: Weak alias to another TLS variable */
extern __thread int tls_weak_alias_target __attribute__((weak));
__thread int tls_weak_alias_target = 123;

#ifdef __GNUC__
__thread int tls_weak_alias __attribute__((weak, alias("tls_weak_alias_target")));
#endif

/* Function that uses all TLS variables to ensure they're not optimized away */
void use_all_tls(void) {
    /* Take addresses and perform operations */
    int *p1 = &tls_public_used;
    int *p2 = &tls_weak;
    int *p3 = &tls_hidden;
    int *p4 = &tls_protected;
    int *p6 = &tls_common;
    int *p7 = &tls_external;
    int *p9 = &tls_internal;
    
    /* Modify some values */
    tls_public_used += 1;
    tls_hidden *= 2;
    tls_internal -= 50;
    
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p6), "r"(p7), "r"(p9));
    
    func_with_tls();
}

/* Main function */
int main(void) {
    use_all_tls();
    
    /* Additional forced usage */
    KEEP_ALIVE(tls_public_used);
    KEEP_ALIVE(tls_weak);
    KEEP_ALIVE(tls_hidden);
    KEEP_ALIVE(tls_protected);
    KEEP_ALIVE(tls_common);
    KEEP_ALIVE(tls_external);
    KEEP_ALIVE(tls_internal);
    
    #ifdef __GNUC__
    KEEP_ALIVE(tls_weak_alias);
    KEEP_ALIVE(tls_weak_alias_target);
    #endif
    
    return 0;
}
