/* Test for TLS emulation attribute copying - Main file */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with explicit initialization and used attribute
   Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__attribute__((used))
__thread int tls_used_public = 42;

/* Test 2: TLS with hidden visibility
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;

/* Test 3: TLS with protected visibility
   Tests: Another visibility variant */
__attribute__((visibility("protected")))
__thread int tls_protected = 200;

/* Test 4: Weak TLS variable (tentative definition)
   Tests: DECL_WEAK, DECL_COMMON */
__attribute__((weak))
__thread int tls_weak;

/* Test 5: External TLS declaration (defined in aux file)
   Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 6: Static TLS with internal visibility
   Tests: Non-public, specific visibility */
static __attribute__((visibility("internal")))
__thread int tls_static_internal = 300;

/* Forward declaration for function in aux file */
void use_tls_variables(void);

/* Function to create DECL_CONTEXT for some variables */
static void function_with_tls(void) {
    /* Test 7: TLS with function context
       Tests: DECL_CONTEXT (non-NULL) */
    static __thread int tls_in_function = 400;
    
    /* Take address to prevent optimization */
    volatile int *p = &tls_in_function;
    (void)p;
}

/* Global weak alias test */
__thread int tls_weak_alias_target = 500;

int main(void) {
    /* Force usage of all TLS variables to prevent elimination */
    
    /* Test 1: Used public TLS */
    tls_used_public += 1;
    asm volatile("" : : "r"(&tls_used_public));
    
    /* Test 2: Hidden TLS */
    tls_hidden = tls_used_public * 2;
    asm volatile("" : : "r"(&tls_hidden));
    
    /* Test 3: Protected TLS */
    tls_protected++;
    asm volatile("" : : "r"(&tls_protected));
    
    /* Test 4: Weak TLS */
    tls_weak = 123;
    asm volatile("" : : "r"(&tls_weak));
    
    /* Test 5: External TLS (defined in aux) */
    tls_external = 456;
    asm volatile("" : : "r"(&tls_external));
    
    /* Test 6: Static internal TLS */
    tls_static_internal = 789;
    asm volatile("" : : "r"(&tls_static_internal));
    
    /* Test 7: Call function with TLS context */
    function_with_tls();
    
    /* Use variables from aux file */
    use_tls_variables();
    
    /* Take address of weak alias target */
    asm volatile("" : : "r"(&tls_weak_alias_target));
    
    return 0;
}

#ifdef __cplusplus
}
#endif
