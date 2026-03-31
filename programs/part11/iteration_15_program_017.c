/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Explicit emulated TLS with various attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;
__thread int tls_not_used = 0;

/* TREE_PUBLIC/DECL_EXTERNAL - public vs static */
__thread int tls_public = 100;
static __thread int tls_static = 200;

/* DECL_COMMON - tentative definition */
__thread int tls_common;  /* Should become common symbol */

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY - various visibility settings */
__thread int tls_default __attribute__((visibility("default"))) = 400;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 500;
__thread int tls_protected __attribute__((visibility("protected"))) = 600;

/* DECL_CONTEXT - different scopes */
static void func_with_tls(void) {
    /* Local static TLS - different context */
    static __thread int tls_local_static = 700;
    tls_local_static++;
}

/* Complex usage to ensure TREE_USED is set */
void use_tls_vars(void) {
    /* Reference all TLS variables to mark them as used */
    tls_used += 1;
    tls_not_used = tls_public + tls_static;
    tls_common = tls_weak;
    tls_default = tls_hidden + tls_protected;
    
    /* Take addresses to force more complex handling */
    int *ptr1 = &tls_used;
    int *ptr2 = &tls_public;
    (void)ptr1;
    (void)ptr2;
    
    func_with_tls();
}

/* Extern declaration to test DECL_EXTERNAL */
extern __thread int tls_extern;

/* Target-specific attributes */
#ifdef _WIN32
/* DECL_DLLIMPORT_P - Windows specific */
extern __thread int __declspec(dllimport) tls_imported;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int __attribute__((dllimport)) tls_imported;
#endif

/* Pattern D: Different TLS models for contrast */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 800;
__thread int tls_emulated __attribute__((tls_model("emulated"))) = 900;
#endif

/* Inline assembly to force special handling */
void asm_use_tls(void) {
    int val;
    __asm__ volatile (
        "movl %1, %0\n\t"
        : "=r" (val)
        : "m" (tls_used)
    );
    (void)val;
}

int main(void) {
    /* Initialize and use TLS variables */
    tls_common = 123;
    tls_not_used = 456;
    
    use_tls_vars();
    asm_use_tls();
    
    /* Calculate sum for observable output */
    int sum = tls_used + tls_public + tls_static + tls_common + 
              tls_weak + tls_default + tls_hidden + tls_protected;
    
#ifdef __GNUC__
    sum += tls_global_dynamic + tls_emulated;
#endif
    
    printf("TLS sum: %d\n", sum);
    return 0;
}
