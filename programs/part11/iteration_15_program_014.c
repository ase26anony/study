/* Test for emulated TLS attribute copying coverage */
#include <stdio.h>
#include <stddef.h>

/* Pattern A: Explicit emulated TLS with various attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_preserve __attribute__((used)) = 42;

/* TREE_PUBLIC + DECL_COMMON - tentative definition (common symbol) */
__thread int tls_common;  /* No initializer at file scope */

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* DECL_VISIBILITY - hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* DECL_VISIBILITY - default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 300;

/* DECL_VISIBILITY - protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* Static TLS (not TREE_PUBLIC) */
static __thread int tls_static = 500;

/* DECL_CONTEXT - function scope TLS */
static void func_with_tls(void) {
    static __thread int tls_func_scope = 600;
    tls_func_scope++;
}

/* DECL_EXTERNAL - external declaration (defined in another file) */
extern __thread int tls_external;

/* Pattern C: Complex usage to ensure processing */
__thread int* tls_ptr;

/* Force TREE_USED by referencing all variables */
void use_all_tls(void) {
    /* Reference all TLS variables to mark them as used */
    tls_preserve++;
    tls_common = 1;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    tls_protected++;
    tls_static++;
    
    /* Take address to force more complex handling */
    tls_ptr = &tls_preserve;
    
    /* Use in expression */
    int sum = tls_preserve + tls_common + tls_weak;
    
    /* Call function with TLS */
    func_with_tls();
}

/* Pattern D: Different TLS model for contrast */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 700;
#endif

int main(void) {
    use_all_tls();
    
    /* Ensure all TLS variables are actually used */
    printf("TLS values:\n");
    printf("tls_preserve: %d\n", tls_preserve);
    printf("tls_common: %d\n", tls_common);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_default: %d\n", tls_default);
    printf("tls_protected: %d\n", tls_protected);
    printf("tls_static: %d\n", tls_static);
    
    /* Reference external TLS */
    printf("tls_external: %d\n", tls_external);
    
    return 0;
}
