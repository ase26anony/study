/* Test for TLS emulation attribute copying - main file */
#include <stddef.h>

/* Prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(&var))

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

/* Test 5: Common TLS (tentative definition) */
__thread int tls_common;
/* Tests: DECL_COMMON */

/* Test 6: External declaration (defined in aux file) */
extern __thread int tls_external;

/* Test 7: Static TLS with internal linkage */
static __thread int tls_static = 7;

/* Function to test TLS in different context */
static void inner_function(void) {
    /* Test 8: TLS with function context */
    static __thread int tls_in_function = 99;
    /* Tests: DECL_CONTEXT (non-NULL) */
    
    tls_in_function++;
    KEEP(tls_in_function);
}

/* For C++ compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* Declaration from aux file */
void use_external_tls(void);

#ifdef __cplusplus
}
#endif

int main(void) {
    /* Use all TLS variables to ensure they're not optimized away */
    
    /* Public used */
    tls_public_used += 1;
    KEEP(tls_public_used);
    
    /* Weak */
    if (&tls_weak != NULL) {
        tls_weak = 1;
    }
    KEEP(tls_weak);
    
    /* Hidden */
    tls_hidden *= 2;
    KEEP(tls_hidden);
    
    /* Protected */
    tls_protected = 50;
    KEEP(tls_protected);
    
    /* Common */
    tls_common = 123;
    KEEP(tls_common);
    
    /* External (defined in aux file) */
    use_external_tls();
    KEEP(tls_external);
    
    /* Static */
    tls_static--;
    KEEP(tls_static);
    
    /* Function context TLS */
    inner_function();
    
    /* Take addresses to force TLS emulation structure creation */
    volatile void* addresses[] = {
        &tls_public_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_common,
        &tls_external,
        &tls_static,
    };
    
    /* Prevent unused variable warning */
    (void)addresses;
    
    return 0;
}
