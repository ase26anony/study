/* Test for TLS emulation attribute copying - Main file */
#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Public TLS with default visibility and used attribute */
__attribute__((used))
__thread int tls_public_used = 42;
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */

/* Test 2: Weak TLS variable */
__attribute__((weak))
__thread int tls_weak;
/* Tests: DECL_WEAK */

/* Test 3: Hidden visibility TLS */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 4: Protected visibility TLS */
__attribute__((visibility("protected")))
__thread int tls_protected;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 5: Static TLS (non-public) inside function for DECL_CONTEXT */
static void func_with_tls(void) {
    /* Local static TLS - will have function as DECL_CONTEXT */
    static __thread int tls_local_static = 7;
    /* Tests: DECL_CONTEXT (non-NULL), !TREE_PUBLIC */
    
    KEEP_ALIVE(tls_local_static);
    tls_local_static++;
}

/* Test 6: External declaration (defined in aux file) */
extern __thread int tls_external;
/* Tests: DECL_EXTERNAL */

/* Test 7: Common TLS (tentative definition) */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Test 8: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use visibility to test similar attribute path */
__attribute__((visibility("default"))) __thread int tls_dllimport;
#endif
/* Tests: DECL_DLLIMPORT_P or fallback visibility */

/* Test 9: Internal visibility */
__attribute__((visibility("internal")))
__thread int tls_internal;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Function that uses all TLS variables */
void use_all_tls(void) {
    /* Take addresses to ensure variables are processed */
    int *ptrs[] = {
        &tls_public_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_external,
        &tls_common,
        &tls_dllimport,
        &tls_internal
    };
    
    /* Modify some values */
    tls_public_used += 1;
    tls_hidden *= 2;
    
    /* Use KEEP_ALIVE on all */
    for (size_t i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
        KEEP_ALIVE(*ptrs[i]);
    }
    
    func_with_tls();
}

/* Main function */
int main(void) {
    use_all_tls();
    
    /* Additional forced usage patterns */
    
    /* Test weak attribute by providing strong definition if weak is unresolved */
    if (&tls_weak == NULL) {
        static __thread int tls_weak_strong = 0;
        KEEP_ALIVE(tls_weak_strong);
    }
    
    /* Test common linkage by multiple tentative definitions */
    tls_common = 123;
    
    /* Force external reference */
    extern void use_external_tls(void);
    use_external_tls();
    
    return 0;
}

/* Force generation of emutls structures even if unused */
__attribute__((constructor))
void init_tls_refs(void) {
    /* Reference all TLS variables in constructor */
    volatile int dummy = tls_public_used + tls_hidden;
    (void)dummy;
}
