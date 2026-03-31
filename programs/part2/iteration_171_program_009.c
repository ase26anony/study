/* Test program for TLS emulation attribute copying coverage */
/* This tests the copy_decl_attributes function in tree-emutls.cc */

#include <stddef.h>

/* Prevent optimization from removing unused TLS variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: TLS variable with used attribute - tests DECL_PRESERVE_P */
__attribute__((used)) __thread int tls_used = 42;

/* Test 2: Public TLS variable - tests TREE_PUBLIC */
__thread int tls_public = 100;

/* Test 3: External TLS declaration (defined in another file) - tests DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 4: Common TLS variable (tentative definition) - tests DECL_COMMON */
__thread int tls_common;

/* Test 5: Weak TLS variable - tests DECL_WEAK */
__attribute__((weak)) __thread int tls_weak = 200;

/* Test 6: TLS with hidden visibility - tests DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 300;

/* Test 7: TLS with protected visibility */
__attribute__((visibility("protected"))) __thread int tls_protected = 400;

/* Test 8: Static TLS (non-public) for contrast */
static __thread int tls_static = 500;

/* Function to create DECL_CONTEXT for some variables */
static void create_context(void) {
    /* Test 9: TLS inside function scope - tests DECL_CONTEXT */
    static __thread int tls_in_function = 600;
    KEEP_ALIVE(tls_in_function);
}

/* For DLL import testing on Windows targets */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#endif

/* Alternative for MinGW/Cygwin */
#if defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_mingw_dllimport;
#endif

/* Weak alias test */
extern __thread int tls_weak_alias_target;
__attribute__((weak, alias("tls_weak_alias_target"))) __thread int tls_weak_alias;

int main(void) {
    /* Ensure all TLS variables are referenced to prevent optimization */
    
    /* Test 1: Used attribute */
    tls_used += 1;
    KEEP_ALIVE(tls_used);
    
    /* Test 2: Public */
    int *p_public = &tls_public;
    KEEP_ALIVE(p_public);
    
    /* Test 3: External (will be defined in emutls_aux.c) */
    tls_external = 123;
    KEEP_ALIVE(tls_external);
    
    /* Test 4: Common */
    tls_common = 456;
    KEEP_ALIVE(tls_common);
    
    /* Test 5: Weak */
    if (&tls_weak) {
        tls_weak = 789;
    }
    KEEP_ALIVE(tls_weak);
    
    /* Test 6: Hidden visibility */
    tls_hidden *= 2;
    KEEP_ALIVE(tls_hidden);
    
    /* Test 7: Protected visibility */
    tls_protected /= 2;
    KEEP_ALIVE(tls_protected);
    
    /* Test 8: Static */
    tls_static = 999;
    KEEP_ALIVE(tls_static);
    
    /* Create context for function-scoped TLS */
    create_context();
    
    /* Test weak alias */
    if (&tls_weak_alias) {
        tls_weak_alias = 111;
    }
    KEEP_ALIVE(tls_weak_alias);
    
    /* Windows-specific tests */
#ifdef _WIN32
    KEEP_ALIVE(tls_dllimport);
#endif
    
#if defined(__MINGW32__) || defined(__CYGWIN__)
    KEEP_ALIVE(tls_mingw_dllimport);
#endif
    
    return 0;
}
