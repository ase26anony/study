/* Test for emulated TLS attribute copying - C version */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Explicit emulated TLS with various attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;
__thread int tls_not_used = 100;

/* TREE_PUBLIC / DECL_EXTERNAL combinations */
__thread int tls_public = 1;                     /* Public, defined */
static __thread int tls_static = 2;              /* Static (not public) */
extern __thread int tls_extern;                  /* External declaration */

/* DECL_COMMON - tentative definition */
__thread int tls_common;                         /* Should be common */

/* DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 3;

/* DECL_VISIBILITY combinations */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 4;
__thread int tls_protected __attribute__((visibility("protected"))) = 5;
__thread int tls_internal __attribute__((visibility("internal"))) = 6;
/* Default visibility is implicit */

/* DECL_CONTEXT - different scopes */
static void function_scope(void) {
    /* Local TLS in function scope */
    static __thread int tls_local_static = 7;
    tls_local_static++;
}

/* DECL_DLLIMPORT_P - target specific */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Pattern C: Complex usage to ensure processing */
volatile int* get_tls_address(void) {
    return &tls_used;
}

void use_tls_complex(void) {
    /* Take addresses, use in expressions */
    int* ptr1 = &tls_public;
    int* ptr2 = &tls_hidden;
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(ptr1), "r"(ptr2));
    
    /* Complex expressions */
    tls_public = tls_hidden + tls_protected;
    tls_common = tls_weak * 2;
}

/* Pattern D: Different TLS models for contrast */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 8;
__thread int tls_local_dynamic __attribute__((tls_model("local-dynamic"))) = 9;
__thread int tls_initial_exec __attribute__((tls_model("initial-exec"))) = 10;
__thread int tls_local_exec __attribute__((tls_model("local-exec"))) = 11;
#endif

int main(void) {
    /* Ensure TREE_USED is set for all variables */
    int sum = 0;
    
    /* Reference all TLS variables */
    sum += tls_used;
    sum += tls_not_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_extern;           /* Defined in another file */
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    
    function_scope();
    use_tls_complex();
    
    /* Reference TLS with different models */
    #ifdef __GNUC__
    sum += tls_global_dynamic;
    sum += tls_local_dynamic;
    sum += tls_initial_exec;
    sum += tls_local_exec;
    #endif
    
    printf("TLS sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
