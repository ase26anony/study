/* Test for emulated TLS attribute copying - C version */

#include <stdio.h>
#include <stdint.h>

/* Pattern A: Explicit emulated TLS model */
#ifdef __cplusplus
extern "C" {
#endif

/* DECL_PRESERVE_P - marked as used */
__thread int tls_preserve __attribute__((used)) = 42;
__thread int tls_not_preserve = 0;

/* TREE_USED - will be referenced */
__thread int tls_used_var = 100;
__thread int tls_unused_var;  /* Not referenced initially */

/* TREE_PUBLIC / DECL_EXTERNAL */
__thread int tls_public = 1;           /* Public, non-static */
static __thread int tls_static = 2;    /* Static, not public */
extern __thread int tls_extern;        /* External declaration */

/* DECL_COMMON - tentative definition */
__thread int tls_common;               /* Should become common symbol */

/* DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 3;
__thread int tls_strong = 4;

/* DECL_VISIBILITY variations */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 5;
__thread int tls_protected __attribute__((visibility("protected"))) = 6;
__thread int tls_internal __attribute__((visibility("internal"))) = 7;
/* Default visibility is implicit */

/* DECL_DLLIMPORT_P - target specific */
#ifdef _WIN32
__declspec(dllimport) extern __thread int tls_dllimport;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
__attribute__((dllimport)) extern __thread int tls_dllimport;
#endif

/* DECL_CONTEXT - different scopes */
static void func_with_tls(void) {
    /* Function scope TLS */
    static __thread int tls_func_scope = 8;
    tls_func_scope++;
}

/* Force usage to set TREE_USED */
void use_tls_vars(void) {
    /* Reference all used variables */
    tls_used_var += 1;
    tls_preserve += 1;
    tls_public += 1;
    tls_static += 1;
    tls_weak += 1;
    tls_strong += 1;
    tls_hidden += 1;
    tls_protected += 1;
    tls_internal += 1;
    tls_common += 1;
    
    /* Take address to force processing */
    int *ptr = &tls_used_var;
    (void)ptr;
    
    func_with_tls();
}

/* Pattern C: Complex expressions with TLS */
int complex_tls_usage(void) {
    /* Use in non-trivial ways */
    int result = tls_used_var;
    
    /* Address taking */
    result += *(&tls_public);
    
    /* Compound assignment */
    tls_weak *= 2;
    
    /* Conditional usage */
    result = (tls_strong > 0) ? tls_strong : tls_weak;
    
    return result;
}

/* Pattern D: Different TLS models mixed */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 9;
__thread int tls_emulated __attribute__((tls_model("emulated"))) = 10;
#endif

int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    tls_unused_var = 50;  /* Now it's used */
    
    use_tls_vars();
    sum += complex_tls_usage();
    
    /* Use all variables to ensure they're processed */
    sum += tls_preserve;
    sum += tls_not_preserve;
    sum += tls_used_var;
    sum += tls_unused_var;
    sum += tls_public;
    sum += tls_static;
    sum += tls_weak;
    sum += tls_strong;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_common;
    
#ifdef __GNUC__
    sum += tls_global_dynamic;
    sum += tls_emulated;
#endif
    
    func_with_tls();
    
    printf("TLS sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}

#ifdef __cplusplus
}
#endif
