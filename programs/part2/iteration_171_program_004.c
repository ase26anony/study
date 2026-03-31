/* Test for TLS emulation attribute copying coverage */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Public TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */
__attribute__((used))
__thread int tls_public_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__attribute__((weak))
__thread int tls_weak = 100;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 200;

/* Test 4: Protected visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("protected")))
__thread int tls_protected = 300;

/* Test 5: Static TLS (non-public) with internal linkage */
/* Tests: !TREE_PUBLIC, DECL_CONTEXT (when in function scope) */
static __thread int tls_static = 400;

/* Test 6: External declaration (defined in emutls_aux.c) */
/* Tests: DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 7: Common TLS (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common;

/* Test 8: DLL import style attribute (for MinGW/Cygwin targets) */
/* Tests: DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Fallback for other targets */
__thread int tls_dllimport;
#endif

/* Function to create local scope TLS */
static void create_local_tls(void) {
    /* Test 9: TLS with function scope - tests DECL_CONTEXT */
    static __thread int tls_local_scope = 500;
    
    /* Take address to prevent optimization */
    volatile int *ptr = &tls_local_scope;
    (void)ptr;
}

/* Function that uses all TLS variables to prevent dead code elimination */
void use_all_tls_vars(void) {
    /* Take addresses and perform operations */
    volatile int *ptrs[] = {
        &tls_public_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_static,
        &tls_external,
        &tls_common,
        &tls_dllimport
    };
    
    /* Modify some values */
    tls_public_used += 1;
    tls_weak += 2;
    tls_hidden += 3;
    tls_protected += 4;
    tls_static += 5;
    tls_common += 6;
    
    /* Use inline asm to ensure variables are marked used */
    asm volatile("" : : "r"(&tls_public_used));
    asm volatile("" : : "r"(&tls_hidden));
    asm volatile("" : : "r"(&tls_protected));
    
    /* Call function with local TLS */
    create_local_tls();
}

/* Main function */
int main(void) {
    /* Initialize coverage */
    int i;
    
    /* Use all TLS variables */
    use_all_tls_vars();
    
    /* Additional operations to ensure coverage */
    for (i = 0; i < 10; i++) {
        tls_public_used += i;
        tls_static -= i;
    }
    
    /* Reference external TLS */
    tls_external = tls_public_used;
    
    /* Return based on TLS values (prevents optimization) */
    return (tls_public_used + tls_static + tls_external) % 256;
}

#ifdef __cplusplus
}
#endif
