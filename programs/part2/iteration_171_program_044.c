/* Test for TLS emulation attribute copying - Main file */
#include <stddef.h>

/* Prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(&var))

/* Test 1: Public TLS with default visibility and used attribute */
__attribute__((used)) __thread int tls_public_used = 42;
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */

/* Test 2: Weak TLS variable */
__attribute__((weak)) __thread int tls_weak;
/* Tests: DECL_WEAK, DECL_COMMON (tentative definition) */

/* Test 3: Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 100;
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* Test 4: Protected visibility with external linkage */
__attribute__((visibility("protected"))) extern __thread int tls_protected;
/* Tests: DECL_EXTERNAL, TREE_PUBLIC, DECL_VISIBILITY */

/* Test 5: Static TLS (non-public) inside function for DECL_CONTEXT */
static void func_with_tls(void) {
    static __thread int tls_in_function = 999;
    /* Tests: DECL_CONTEXT (function scope), !TREE_PUBLIC */
    KEEP(tls_in_function);
}

/* Test 6: Common TLS (tentative definition) with internal visibility */
__attribute__((visibility("internal"))) __thread int tls_internal;
/* Tests: DECL_COMMON, DECL_VISIBILITY */

/* Forward declaration for function in auxiliary file */
void use_tls_variables(void);

int main(void) {
    /* Ensure all TLS variables are referenced to prevent elimination */
    
    /* Test 1: Public used TLS */
    tls_public_used += 1;
    KEEP(tls_public_used);
    
    /* Test 2: Weak TLS */
    tls_weak = 123;
    KEEP(tls_weak);
    
    /* Test 3: Hidden TLS */
    int *hidden_ptr = &tls_hidden;
    *hidden_ptr += 1;
    KEEP(tls_hidden);
    
    /* Test 4: Protected external TLS (defined in aux file) */
    extern __thread int tls_protected;
    tls_protected = 456;
    KEEP(tls_protected);
    
    /* Test 5: Function-local static TLS */
    func_with_tls();
    
    /* Test 6: Internal TLS */
    tls_internal = 789;
    KEEP(tls_internal);
    
    /* Test 7: DLL import style (simulated with weak alias) */
    extern __thread int tls_aliased;
    tls_aliased = 111;
    KEEP(tls_aliased);
    
    /* Call function from other compilation unit */
    use_tls_variables();
    
    return 0;
}

/* Force emission of TLS variables even if unused in dead code elimination */
__attribute__((constructor)) void mark_tls_used(void) {
    /* Take addresses to ensure TLS vars are emitted */
    volatile int *ptrs[] = {
        &tls_public_used,
        &tls_weak,
        &tls_hidden,
        &tls_internal,
    };
    (void)ptrs; /* Suppress unused warning */
}
