/* Test for TLS emulation attribute copying coverage */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Attribute availability checks */
#ifdef __GNUC__
#define HAVE_ATTRIBUTE(x) 1
#else
#define HAVE_ATTRIBUTE(x) 0
#endif

/* Test 1: Public TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED, DECL_VISIBILITY(default) */
#if HAVE_ATTRIBUTE(used)
__attribute__((used))
#endif
__thread int tls_public_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
#if HAVE_ATTRIBUTE(weak)
__attribute__((weak))
#endif
__thread int tls_weak_var;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY(hidden), DECL_VISIBILITY_SPECIFIED */
#if HAVE_ATTRIBUTE(visibility)
__attribute__((visibility("hidden")))
#endif
__thread int tls_hidden = 100;

/* Test 4: Protected visibility TLS */
/* Tests: DECL_VISIBILITY(protected), DECL_VISIBILITY_SPECIFIED */
#if HAVE_ATTRIBUTE(visibility)
__attribute__((visibility("protected")))
#endif
__thread int tls_protected = 200;

/* Test 5: Static TLS (non-public) inside a function scope */
/* Tests: DECL_CONTEXT (function scope), !TREE_PUBLIC */
static void test_function_scope(void) {
    static __thread int tls_function_static = 300;
    KEEP_ALIVE(tls_function_static);
}

/* Test 6: External TLS declaration (defined in emutls_aux.c) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 7: Common TLS (tentative definition) */
/* Tests: DECL_COMMON, TREE_PUBLIC */
__thread int tls_common;

/* Test 8: DLL import style attribute (for MinGW/Cygwin targets) */
/* Tests: DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Simulate with visibility on non-Windows */
__attribute__((visibility("default"))) __thread int tls_dllimport = 500;
#endif

/* Test 9: Internal visibility */
/* Tests: DECL_VISIBILITY(internal), DECL_VISIBILITY_SPECIFIED */
#if HAVE_ATTRIBUTE(visibility)
__attribute__((visibility("internal")))
#endif
__thread int tls_internal = 600;

/* Function that uses all TLS variables to ensure they're processed */
void use_all_tls_vars(void) {
    /* Take addresses to prevent optimization */
    int *ptrs[] = {
        &tls_public_used,
        &tls_weak_var,
        &tls_hidden,
        &tls_protected,
        &tls_external,
        &tls_common,
        &tls_dllimport,
        &tls_internal
    };
    
    /* Use volatile asm to keep pointers alive */
    for (size_t i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
        KEEP_ALIVE(ptrs[i]);
    }
    
    /* Read-modify-write on some variables */
    tls_public_used += 1;
    tls_hidden *= 2;
    tls_protected -= 5;
    
    /* Call function with static TLS */
    test_function_scope();
}

/* Function from auxiliary file */
extern void aux_function(void);

int main(void) {
    /* Initialize common TLS */
    tls_common = 700;
    
    /* Use all TLS variables */
    use_all_tls_vars();
    
    /* Call function from other compilation unit */
    aux_function();
    
    /* Final use to ensure all paths are taken */
    tls_public_used += tls_external;
    
    return 0;
}
