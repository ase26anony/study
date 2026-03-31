/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Explicit emulated TLS model */
#ifdef __GNUC__
#define EMULATED_TLS __attribute__((tls_model("emulated")))
#else
#define EMULATED_TLS
#endif

/* DECL_PRESERVE_P: Variables that should not be eliminated */
__thread int tls_preserved EMULATED_TLS __attribute__((used));
__thread int tls_not_preserved EMULATED_TLS;

/* TREE_USED: Variables that are referenced */
__thread int tls_used_global EMULATED_TLS = 42;
static __thread int tls_used_static EMULATED_TLS = 100;

/* TREE_PUBLIC / DECL_EXTERNAL: Mix of public and static */
__thread int tls_public EMULATED_TLS = 1;           /* Public */
static __thread int tls_static EMULATED_TLS = 2;    /* Static (not public) */

/* DECL_COMMON: Tentative definitions */
__thread int tls_common;  /* Should become common symbol */
__thread int tls_common2 EMULATED_TLS;

/* DECL_WEAK: Weak symbols */
__thread int tls_weak EMULATED_TLS __attribute__((weak)) = 3;
extern __thread int tls_weak_extern EMULATED_TLS __attribute__((weak));

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_default_vis EMULATED_TLS __attribute__((visibility("default"))) = 10;
__thread int tls_hidden_vis EMULATED_TLS __attribute__((visibility("hidden"))) = 20;
__thread int tls_protected_vis EMULATED_TLS __attribute__((visibility("protected"))) = 30;

/* DECL_DLLIMPORT_P: DLL import attributes (Windows-specific) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function-scoped TLS (tests DECL_CONTEXT) */
void test_function_context(void) {
    static __thread int tls_in_function EMULATED_TLS = 99;
    tls_in_function++;
    printf("Function TLS: %d\n", tls_in_function);
}

/* Complex usage patterns to ensure processing */
void complex_tls_usage(void) {
    /* Address taking */
    int *ptr = &tls_used_global;
    
    /* Using in expressions */
    tls_used_static = tls_used_global * 2;
    
    /* Asm reference (ensures variable is marked used) */
    asm volatile("" : "+m" (tls_used_global));
    
    /* Reference weak symbol */
    if (&tls_weak_extern) {
        /* Do nothing, just reference */
    }
}

/* Force TREE_USED on all variables */
void reference_all_vars(void) {
    /* Reference all TLS variables to ensure they're marked used */
    tls_preserved = 1;
    tls_not_preserved = 2;
    tls_public = 3;
    tls_static = 4;
    tls_common = 5;
    tls_common2 = 6;
    tls_weak = 7;
    tls_default_vis = 8;
    tls_hidden_vis = 9;
    tls_protected_vis = 10;
    
    /* Take addresses */
    void *addrs[] = {
        &tls_preserved,
        &tls_not_preserved,
        &tls_public,
        &tls_static,
        &tls_common,
        &tls_common2,
        &tls_weak,
        &tls_default_vis,
        &tls_hidden_vis,
        &tls_protected_vis
    };
    
    /* Use in computation */
    int sum = tls_preserved + tls_not_preserved + tls_public + tls_static +
              tls_common + tls_common2 + tls_weak +
              tls_default_vis + tls_hidden_vis + tls_protected_vis;
    
    printf("TLS sum: %d\n", sum);
}

int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initialize and use all TLS variables */
    reference_all_vars();
    
    /* Test function-scoped TLS */
    test_function_context();
    test_function_context();  /* Call twice to ensure persistence */
    
    /* Complex usage */
    complex_tls_usage();
    
    /* Verify values */
    printf("tls_used_global = %d\n", tls_used_global);
    printf("tls_used_static = %d\n", tls_used_static);
    
    return 0;
}
