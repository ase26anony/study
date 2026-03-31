/* Test for TLS emulation attribute copying - main file */
#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&var))

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

/* Test 5: Common TLS (tentative definition) */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Test 6: External TLS declaration (defined in aux file) */
extern __thread int tls_external;

/* Test 7: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Use weak as approximation for non-Windows */
__attribute__((weak)) __thread int tls_dllimport;
#endif
/* Tests: DECL_DLLIMPORT_P or DECL_WEAK */

/* Test 8: TLS with context (inside function) */
static void func_with_tls(void) {
    /* Local TLS with context */
    static __thread int tls_with_context = 99;
    /* Tests: DECL_CONTEXT (non-NULL) */
    KEEP_ALIVE(tls_with_context);
}

/* Test 9: TLS with internal visibility */
__attribute__((visibility("internal")))
__thread int tls_internal;

/* Function that uses all TLS variables to prevent optimization */
void use_all_tls(void) {
    /* Take addresses and perform operations */
    int *ptr;
    
    ptr = &tls_public_used;
    tls_public_used += 1;
    
    ptr = &tls_weak;
    *ptr = 10;
    
    ptr = &tls_hidden;
    tls_hidden *= 2;
    
    ptr = &tls_protected;
    tls_protected = 50;
    
    ptr = &tls_common;
    tls_common = 30;
    
    ptr = &tls_external;
    /* Just take address, value set in aux file */
    
    ptr = &tls_dllimport;
    *ptr = 70;
    
    ptr = &tls_internal;
    tls_internal = 80;
    
    func_with_tls();
}

/* Main function */
int main(void) {
    /* Force all TLS variables to be processed */
    use_all_tls();
    
    /* Additional keep-alive for coverage */
    KEEP_ALIVE(tls_public_used);
    KEEP_ALIVE(tls_weak);
    KEEP_ALIVE(tls_hidden);
    KEEP_ALIVE(tls_protected);
    KEEP_ALIVE(tls_common);
    KEEP_ALIVE(tls_external);
    KEEP_ALIVE(tls_dllimport);
    KEEP_ALIVE(tls_internal);
    
    return 0;
}
