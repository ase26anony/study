/* Test for emulated TLS attribute copying - C version */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Explicit emulated TLS model */
#ifdef __GNUC__
#define EMULATED_TLS __attribute__((tls_model("emulated")))
#else
#define EMULATED_TLS
#endif

/* 1. DECL_PRESERVE_P - variables that must not be eliminated */
__thread int tls_preserved EMULATED_TLS __attribute__((used));
__thread int tls_not_preserved EMULATED_TLS;

/* 2. TREE_USED - variables that are referenced */
__thread int tls_used EMULATED_TLS = 42;
__thread int tls_unused EMULATED_TLS;

/* 3. TREE_PUBLIC / DECL_EXTERNAL - linkage variations */
__thread int tls_public EMULATED_TLS = 100;           /* public */
static __thread int tls_static EMULATED_TLS = 200;    /* not public */
extern __thread int tls_external EMULATED_TLS;        /* external declaration */

/* 4. DECL_COMMON - tentative definitions */
__thread int tls_common;                              /* common symbol */
__thread int tls_initialized = 300;                   /* not common */

/* 5. DECL_WEAK - weak symbols */
__thread int tls_weak EMULATED_TLS __attribute__((weak)) = 400;
__thread int tls_strong EMULATED_TLS = 500;

/* 6. DECL_VISIBILITY - visibility attributes */
__thread int tls_hidden EMULATED_TLS __attribute__((visibility("hidden"))) = 600;
__thread int tls_default EMULATED_TLS __attribute__((visibility("default"))) = 700;
__thread int tls_protected EMULATED_TLS __attribute__((visibility("protected"))) = 800;
__thread int tls_internal EMULATED_TLS __attribute__((visibility("internal"))) = 900;

/* 7. DECL_DLLIMPORT_P - DLL import attributes (Windows targets) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* 8. DECL_CONTEXT - different scopes */
static void function_scope(void) {
    /* Local TLS variable */
    static __thread int tls_local EMULATED_TLS = 1000;
    tls_local++;
}

/* Helper function to ensure variables are used */
void use_tls_variables(void) {
    /* Ensure all TLS variables are marked as used */
    tls_preserved = 1;
    tls_not_preserved = 2;
    tls_used += 1;
    tls_unused = tls_used;
    tls_public = tls_static + 1;
    tls_common = 50;
    tls_weak = tls_strong + 1;
    tls_hidden = tls_default;
    tls_protected = tls_internal;
    
    /* Take addresses to force more complex handling */
    int *ptr1 = &tls_preserved;
    int *ptr2 = &tls_hidden;
    volatile int sink = *ptr1 + *ptr2;
    (void)sink;
    
    function_scope();
}

/* Complex expression using TLS variables */
int complex_tls_expression(void) {
    return tls_used + tls_public + tls_static + tls_common +
           tls_weak + tls_strong + tls_hidden + tls_default +
           tls_protected + tls_internal;
}

int main(void) {
    int result = 0;
    
    /* Initialize and use all TLS variables */
    use_tls_variables();
    
    /* Use in non-trivial ways */
    result = complex_tls_expression();
    
    /* Use in inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_used), "r"(&tls_public));
    
    printf("Result: %d\n", result);
    return 0;
}
