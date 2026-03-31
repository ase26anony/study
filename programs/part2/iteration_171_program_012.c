/* Test program for TLS emulation attribute copying coverage */
/* This file tests various TLS variable attributes to trigger copy_decl_attributes */

#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Public TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */
__attribute__((used))
__thread int tls_public_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, DECL_COMMON (tentative definition) */
__attribute__((weak))
__thread int tls_weak;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;

/* Test 4: Protected visibility TLS */
__attribute__((visibility("protected")))
__thread int tls_protected;

/* Test 5: Static TLS (non-public) with internal linkage */
/* Tests: !TREE_PUBLIC, has DECL_CONTEXT */
static __thread int tls_static_internal;

/* Test 6: External declaration (defined in another file) */
/* Tests: DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 7: Common TLS (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common;

/* Test 8: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
/* Simulate with weak external */
__attribute__((weak)) extern __thread int tls_dllimport;
#endif

/* Function with local TLS to test DECL_CONTEXT */
static void test_local_tls(void) {
    /* Test 9: TLS with function context */
    /* Tests: DECL_CONTEXT (non-NULL) */
    static __thread int tls_in_function;
    tls_in_function++;
    KEEP_ALIVE(tls_in_function);
}

/* Another function to test visibility in different context */
__attribute__((visibility("default")))
static void visibility_test(void) {
    /* This doesn't directly affect TLS but ensures visibility attributes work */
}

int main(void) {
    int result = 0;
    
    /* Use all TLS variables to prevent optimization */
    
    /* Test 1: Public used TLS */
    tls_public_used += 1;
    result += tls_public_used;
    KEEP_ALIVE(tls_public_used);
    
    /* Test 2: Weak TLS */
    if (&tls_weak != NULL) {
        tls_weak = 10;
        result += tls_weak;
    }
    KEEP_ALIVE(tls_weak);
    
    /* Test 3: Hidden TLS */
    tls_hidden *= 2;
    result += tls_hidden;
    KEEP_ALIVE(tls_hidden);
    
    /* Test 4: Protected TLS */
    tls_protected = 50;
    result += tls_protected;
    KEEP_ALIVE(tls_protected);
    
    /* Test 5: Static internal TLS */
    tls_static_internal = 20;
    result += tls_static_internal;
    KEEP_ALIVE(tls_static_internal);
    
    /* Test 6: External TLS (defined in aux file) */
    /* We'll take address even if not defined yet */
    if (&tls_external != NULL) {
        result += 1;
    }
    KEEP_ALIVE(tls_external);
    
    /* Test 7: Common TLS */
    tls_common = 30;
    result += tls_common;
    KEEP_ALIVE(tls_common);
    
    /* Test 8: DLL import style */
#ifdef _WIN32
    if (&tls_dllimport != NULL) {
        result += 2;
    }
#endif
    KEEP_ALIVE(tls_dllimport);
    
    /* Test 9: Local TLS in function */
    test_local_tls();
    
    /* Also test taking addresses in different ways */
    void* addresses[] = {
        &tls_public_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_static_internal,
        &tls_external,
        &tls_common,
#ifdef _WIN32
        &tls_dllimport,
#endif
        NULL
    };
    
    /* Force compiler to consider all addresses */
    for (size_t i = 0; addresses[i] != NULL; i++) {
        result += ((size_t)addresses[i] & 1);
    }
    
    return result > 0 ? 0 : 1;
}

/* C++ compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* Additional TLS in extern "C" block for C++ testing */
__thread int tls_extern_c = 99;

#ifdef __cplusplus
}
#endif
