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

/* Test 6: External TLS declaration (defined in aux file) */
extern __thread int tls_external;
/* Tests: DECL_EXTERNAL */

/* Test 7: Static TLS with internal linkage */
static __thread int tls_static = 7;
/* Tests: DECL_CONTEXT (function context), !TREE_PUBLIC */

/* Test 8: DLL import style (simulated with weak external) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Use weak external to simulate similar behavior */
extern __thread int tls_dllimport __attribute__((weak));
#endif
/* Tests: DECL_DLLIMPORT_P or DECL_WEAK */

/* Function to create context for static TLS */
static void create_tls_context(void) {
    /* tls_static has DECL_CONTEXT set to this function */
    tls_static++;
}

/* External function from aux file */
void use_aux_tls(void);

int main(void) {
    /* Ensure all TLS variables are referenced */
    
    /* Public used */
    tls_public_used += 1;
    KEEP(tls_public_used);
    
    /* Weak */
    tls_weak = 10;
    KEEP(tls_weak);
    
    /* Hidden */
    tls_hidden *= 2;
    KEEP(tls_hidden);
    
    /* Protected */
    tls_protected = 50;
    KEEP(tls_protected);
    
    /* Common */
    tls_common = 99;
    KEEP(tls_common);
    
    /* External */
    tls_external = 123;
    KEEP(tls_external);
    
    /* Static with context */
    create_tls_context();
    KEEP(tls_static);
    
    /* DLL import style */
    tls_dllimport = 456;
    KEEP(tls_dllimport);
    
    /* Use aux file TLS */
    use_aux_tls();
    
    return 0;
}
