/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

#include <stdio.h>
#include <stdint.h>

/* Pattern A: Basic TLS variables with various attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;
__thread int tls_not_used = 100;

/* TREE_PUBLIC/DECL_EXTERNAL - public vs static */
__thread int tls_public = 1;
static __thread int tls_static = 2;

/* DECL_COMMON - tentative definition */
__thread int tls_common;  /* Should become common symbol */

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 3;

/* DECL_VISIBILITY - various visibility settings */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 4;
__thread int tls_protected __attribute__((visibility("protected"))) = 5;
__thread int tls_internal __attribute__((visibility("internal"))) = 6;
/* Default visibility is implicit */

/* DECL_DLLIMPORT_P - dllimport on supported targets */
#ifdef _WIN32
extern __thread int tls_imported __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_imported __attribute__((dllimport));
#endif

/* Pattern C: Complex usage to ensure processing */
static void use_tls_vars(void) {
    /* Ensure TREE_USED is set for all variables */
    volatile int sum = 0;
    
    sum += tls_used;
    sum += tls_not_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    
    /* Take addresses to force additional processing */
    void *addrs[] = {
        &tls_used,
        &tls_not_used,
        &tls_public,
        &tls_static,
        &tls_common,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_internal,
    };
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(sum), "r"(addrs) : "memory");
}

/* Pattern D: Different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 7;
__thread int tls_local_dynamic __attribute__((tls_model("local-dynamic"))) = 8;
__thread int tls_initial_exec __attribute__((tls_model("initial-exec"))) = 9;
__thread int tls_local_exec __attribute__((tls_model("local-exec"))) = 10;

/* DECL_CONTEXT: Variables in different scopes */
static void function_scope(void) {
    /* Function-local TLS */
    static __thread int tls_func_local = 11;
    tls_func_local++;
}

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 12;
    
    /* Use all TLS variables */
    use_tls_vars();
    function_scope();
    
    /* Complex expressions with TLS */
    tls_public = tls_used + tls_not_used;
    tls_hidden = tls_protected * tls_internal;
    
    /* Use different TLS model variables */
    int result = tls_global_dynamic + tls_local_dynamic + 
                 tls_initial_exec + tls_local_exec;
    
    printf("Result: %d\n", result);
    return 0;
}
