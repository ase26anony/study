/* Test for emulated TLS attribute copying coverage */
/* Compile with: gcc -ftls-model=emulated -fPIC tls_main.c tls_aux.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* DECL_PRESERVE_P: Used attribute ensures preservation */
__thread int tls_used __attribute__((used));
__thread int tls_not_used;

/* TREE_USED: Variables that will be referenced */
__thread int tls_referenced = 42;
__thread int tls_unreferenced;

/* TREE_PUBLIC/DECL_EXTERNAL: Public vs static linkage */
__thread int tls_public = 100;
static __thread int tls_static = 200;

/* DECL_COMMON: Tentative definitions (common symbols) */
__thread int tls_common;  /* Tentative definition */
__thread int tls_initialized = 300;  /* Not common */

/* DECL_WEAK: Weak symbols */
__thread int tls_weak __attribute__((weak)) = 400;
__thread int tls_strong = 500;

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 600;
__thread int tls_default __attribute__((visibility("default"))) = 700;
__thread int tls_protected __attribute__((visibility("protected"))) = 800;
__thread int tls_internal __attribute__((visibility("internal"))) = 900;

/* DECL_CONTEXT: Different scopes */
static void function_scope(void) {
    /* TLS in function scope */
    static __thread int tls_function_scope = 1000;
    tls_function_scope++;
}

/* Extern declaration from another file */
extern __thread int tls_extern;
extern __thread int tls_dllimport_var;

/* DECL_DLLIMPORT_P: DLL import attribute (conditional) */
#ifdef _WIN32
extern __thread int __declspec(dllimport) tls_dllimport;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int __attribute__((dllimport)) tls_dllimport;
#endif

/* Force TREE_USED by referencing variables */
void reference_all_tls(void) {
    /* Reference all TLS variables to ensure TREE_USED is set */
    volatile int sum = 0;
    
    sum += tls_used;
    sum += tls_not_used;
    sum += tls_referenced;
    sum += tls_unreferenced;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_initialized;
    sum += tls_weak;
    sum += tls_strong;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_extern;
    
#ifdef _WIN32
    sum += tls_dllimport;
#endif
    
    /* Take addresses to force more complex handling */
    void *addrs[] = {
        &tls_used,
        &tls_referenced,
        &tls_public,
        &tls_weak,
        &tls_hidden,
        &tls_extern
    };
    
    (void)sum;
    (void)addrs;
}

/* Complex expression with TLS */
int complex_tls_expression(void) {
    return tls_referenced * 2 + tls_public / 2 - tls_weak;
}

/* Use TLS in inline assembly to force special handling */
void tls_in_asm(void) {
    int val;
    __asm__ volatile (
        "movl %1, %0\n\t"
        : "=r" (val)
        : "m" (tls_referenced)
    );
    (void)val;
}

/* Non-inlinable function using TLS */
__attribute__((noinline)) 
int use_tls_in_function(int x) {
    return tls_referenced + x + tls_public;
}

/* Different TLS models for comparison */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 1100;
__thread int tls_emulated __attribute__((tls_model("emulated"))) = 1200;

int main(void) {
    /* Initialize and use TLS variables */
    tls_used = 1;
    tls_not_used = 2;
    tls_referenced = 3;
    tls_public = 4;
    tls_static = 5;
    tls_common = 6;
    tls_weak = 7;
    tls_strong = 8;
    tls_hidden = 9;
    tls_default = 10;
    tls_protected = 11;
    tls_internal = 12;
    
    /* Reference all variables */
    reference_all_tls();
    
    /* Use in complex ways */
    int result = complex_tls_expression();
    tls_in_asm();
    result += use_tls_in_function(42);
    
    /* Function scope TLS */
    function_scope();
    
    /* Print something to prevent optimization */
    printf("TLS test result: %d\n", result + tls_extern);
    
    return 0;
}
