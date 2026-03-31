/* Test for TLS emulation attribute copying - main file */
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

/* Test 4: Protected visibility with external linkage */
__attribute__((visibility("protected")))
extern __thread int tls_external;
/* Tests: DECL_EXTERNAL, DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 5: Common TLS (tentative definition) */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Test 6: Static TLS with internal linkage */
static __thread int tls_static = 999;
/* Tests: DECL_CONTEXT (function/block scope), !TREE_PUBLIC */

/* Test 7: DLL import style (using weak as proxy) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* On non-Windows, use weak attribute as proxy for DLL import behavior */
__attribute__((weak))
__thread int tls_dllimport;
#endif
/* Tests: DECL_DLLIMPORT_P or DECL_WEAK */

/* Function to test TLS in different context */
static void test_function_context(void) {
    /* Test 8: TLS with function context */
    static __thread int tls_function_local = 1234;
    /* Tests: DECL_CONTEXT (non-NULL) */
    
    tls_function_local++;
    KEEP_ALIVE(tls_function_local);
}

/* External function from aux file */
extern void use_external_tls(void);

int main(void) {
    /* Ensure all TLS variables are referenced */
    
    /* Test 1 */
    tls_public_used += 1;
    KEEP_ALIVE(tls_public_used);
    
    /* Test 2 */
    tls_weak = 50;
    KEEP_ALIVE(tls_weak);
    
    /* Test 3 */
    int *hidden_ptr = &tls_hidden;
    KEEP_ALIVE(hidden_ptr);
    
    /* Test 4 - will be defined in aux file */
    tls_external = 200;
    KEEP_ALIVE(tls_external);
    
    /* Test 5 */
    tls_common = 300;
    KEEP_ALIVE(tls_common);
    
    /* Test 6 */
    tls_static *= 2;
    KEEP_ALIVE(tls_static);
    
    /* Test 7 */
    tls_dllimport = 400;
    KEEP_ALIVE(tls_dllimport);
    
    /* Test function context TLS */
    test_function_context();
    
    /* Use external TLS variables */
    use_external_tls();
    
    return 0;
}

/* Force compiler to consider all TLS variables */
void __attribute__((used)) keep_all_tls(void) {
    volatile int dummy = 0;
    dummy += tls_public_used;
    dummy += tls_weak;
    dummy += tls_hidden;
    dummy += tls_external;
    dummy += tls_common;
    dummy += tls_static;
    dummy += tls_dllimport;
    (void)dummy;
}
