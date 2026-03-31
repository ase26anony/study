/* Test for emulated TLS attribute copying - Main file */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* Pattern A: Explicit emulated TLS model */
__thread int tls_explicit_emulated __attribute__((tls_model("emulated")));

/* Pattern B: DECL_PRESERVE_P - marked as used */
__thread int tls_preserved __attribute__((used));

/* TREE_USED - will be referenced in code */
__thread int tls_used_var;

/* TREE_PUBLIC - non-static (public) TLS */
__thread int tls_public = 42;

/* Static TLS (not TREE_PUBLIC) */
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

/* DECL_VISIBILITY_SPECIFIED - internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* DECL_CONTEXT - TLS in function scope */
static void func_with_tls(void) {
    static __thread int tls_func_scope = 700;
    tls_func_scope++;
}

/* DECL_CONTEXT - TLS in block scope */
void block_scope_tls(void) {
    {
        __thread int tls_block_scope = 800;
        tls_block_scope = 801;
    }
}

/* Complex usage patterns for Pattern C */
__thread int* tls_pointer;
__thread int tls_for_asm;

/* Extern declaration for multi-file testing */
extern __thread int tls_extern;
extern __thread int tls_shared;

/* Function that uses TLS variables to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    tls_used_var = 1;
    tls_public += 1;
    tls_static *= 2;
    tls_common = tls_public + tls_static;
    tls_weak -= 1;
    tls_hidden++;
    tls_default--;
    tls_protected *= 3;
    tls_internal /= 2;
    
    /* Take address to force more complex handling */
    tls_pointer = &tls_used_var;
    
    /* Use in asm to prevent optimization */
    asm volatile ("" : : "r"(&tls_for_asm));
    
    /* Use extern TLS */
    tls_extern = 999;
    tls_shared = 888;
    
    /* Call function with scope TLS */
    func_with_tls();
    block_scope_tls();
}

/* Weak alias test */
__thread int tls_original = 1234;
extern __thread int tls_alias __attribute__((weak, alias("tls_original")));

int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    tls_explicit_emulated = 10;
    sum += tls_explicit_emulated;
    
    tls_preserved = 20;
    sum += tls_preserved;
    
    use_tls_variables();
    
    /* Sum all TLS values */
    sum += tls_used_var;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_extern;
    sum += tls_shared;
    sum += tls_original;
    sum += tls_alias;
    
    /* Use tls_for_asm */
    tls_for_asm = sum;
    
    printf("TLS sum: %d\n", sum);
    
    return 0;
}
