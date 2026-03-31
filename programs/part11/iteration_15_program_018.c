/* Test for emulated TLS attribute copying coverage */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Explicit emulated TLS model */
__thread int tls_emulated_explicit __attribute__((tls_model("emulated")));

/* Pattern B: Various attributes on TLS variables */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_preserved __attribute__((used));

/* TREE_PUBLIC - non-static (public) TLS */
__thread int tls_public = 42;

/* Static TLS (not public) */
static __thread int tls_static = 100;

/* DECL_COMMON - tentative definition */
__thread int tls_common;

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 200;

/* DECL_VISIBILITY - hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* DECL_VISIBILITY - default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 400;

/* DECL_VISIBILITY - protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* Complex usage to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    tls_emulated_explicit = 1;
    tls_preserved = 2;
    tls_public = tls_public + 1;
    tls_static = tls_static * 2;
    tls_common = 50;
    
    if (&tls_weak) {
        tls_weak = 201;
    }
    
    tls_hidden = tls_hidden + 10;
    tls_default = tls_default - 1;
    tls_protected = tls_protected / 2;
    
    /* Take addresses to force more complex handling */
    volatile int *ptr1 = &tls_emulated_explicit;
    volatile int *ptr2 = &tls_preserved;
    (void)ptr1;
    (void)ptr2;
}

/* Function with local static TLS */
void func_with_local_tls(void) {
    static __thread int local_tls = 999;
    local_tls++;
    printf("Local TLS: %d\n", local_tls);
}

/* Extern declaration for cross-file testing */
extern __thread int tls_extern;

int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    tls_emulated_explicit = 10;
    tls_preserved = 20;
    tls_public = 30;
    tls_static = 40;
    tls_common = 50;
    tls_weak = 60;
    tls_hidden = 70;
    tls_default = 80;
    tls_protected = 90;
    
    /* Use the function that references TLS */
    use_tls_variables();
    func_with_local_tls();
    
    /* Sum all values to ensure they're used */
    sum += tls_emulated_explicit;
    sum += tls_preserved;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_protected;
    
    printf("TLS sum: %d\n", sum);
    
    /* Test with asm to force special handling */
    __asm__ volatile ("" : : "r"(&tls_public) : "memory");
    
    return sum > 0 ? 0 : 1;
}
