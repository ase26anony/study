/* Test program for TLS emulation attribute copying */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Helper macro for attribute compatibility */
#if defined(__GNUC__) || defined(__clang__)
#define HAVE_ATTRIBUTES 1
#else
#define HAVE_ATTRIBUTES 0
#endif

/* Test 1: Basic TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
#if HAVE_ATTRIBUTES
__attribute__((used))
#endif
__thread int tls_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
#if HAVE_ATTRIBUTES
__attribute__((weak))
#endif
__thread int tls_weak;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, TREE_PUBLIC */
#if HAVE_ATTRIBUTES
__attribute__((visibility("hidden")))
#endif
__thread int tls_hidden = 100;

/* Test 4: Protected visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
#if HAVE_ATTRIBUTES
__attribute__((visibility("protected")))
#endif
__thread int tls_protected;

/* Test 5: External declaration (defined in emutls_aux.c) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 6: Common linkage (tentative definition) */
/* Tests: DECL_COMMON, TREE_PUBLIC */
__thread int tls_common;

/* Test 7: Static TLS (non-public) for contrast */
/* Tests: !TREE_PUBLIC, DECL_CONTEXT (when in function scope) */
static __thread int tls_static = 999;

/* Test 8: DLL import simulation (using weak alias pattern) */
/* Tests: DECL_WEAK, DECL_DLLIMPORT_P (on some targets) */
#if HAVE_ATTRIBUTES && (defined(__MINGW32__) || defined(__CYGWIN__))
__declspec(dllimport) __thread int tls_dllimport;
#elif HAVE_ATTRIBUTES
/* Use weak alias to simulate similar behavior */
__thread int tls_dllimport_target = 456;
extern __thread int tls_dllimport 
#if defined(__GNUC__)
    __attribute__((weak, alias("tls_dllimport_target")));
#else
    ;
#endif
#else
extern __thread int tls_dllimport;
#endif

/* Function to test TLS variables in different scope */
static void test_function_scope(void) {
    /* Test 9: TLS with function context */
    /* Tests: DECL_CONTEXT (non-NULL) */
    static __thread int tls_in_function = 777;
    
    /* Use the variable to prevent optimization */
    tls_in_function += 1;
    (void)tls_in_function;
}

/* External function from emutls_aux.c */
extern void use_external_tls(void);

int main(void) {
    /* Force all TLS variables to be processed */
    
    /* Test 1: Used TLS */
    tls_used += 1;
    
    /* Test 2: Weak TLS */
    if (&tls_weak != NULL) {
        tls_weak = 123;
    }
    
    /* Test 3: Hidden TLS */
    tls_hidden *= 2;
    
    /* Test 4: Protected TLS */
    tls_protected = 888;
    
    /* Test 5: External TLS */
    tls_external = 555;
    
    /* Test 6: Common TLS */
    tls_common = 333;
    
    /* Test 7: Static TLS */
    tls_static -= 50;
    
    /* Test 8: DLL import style TLS */
#if HAVE_ATTRIBUTES && !(defined(__MINGW32__) || defined(__CYGWIN__))
    tls_dllimport_target = 111;
#else
    tls_dllimport = 111;
#endif
    
    /* Test 9: Function scope TLS */
    test_function_scope();
    
    /* Take addresses to ensure variables aren't optimized out */
    volatile void* addrs[] = {
        &tls_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_external,
        &tls_common,
        &tls_static,
#if HAVE_ATTRIBUTES && !(defined(__MINGW32__) || defined(__CYGWIN__))
        &tls_dllimport_target,
#else
        &tls_dllimport,
#endif
    };
    (void)addrs; /* Suppress unused warning */
    
    /* Use inline asm to mark variables as used (for DECL_PRESERVE_P) */
#if HAVE_ATTRIBUTES && (defined(__GNUC__) || defined(__clang__))
    __asm__ volatile ("" : : "r"(&tls_used));
    __asm__ volatile ("" : : "r"(&tls_hidden));
#endif
    
    /* Call external function that uses TLS */
    use_external_tls();
    
    return 0;
}

#ifdef __cplusplus
}
#endif
