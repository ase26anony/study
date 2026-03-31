/* tls_main.c - Main test file for emulated TLS attribute coverage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS for all variables */
#pragma GCC tls_model emulated

/* ===== DECL_PRESERVE_P testing ===== */
/* Variable with __attribute__((used)) to ensure DECL_PRESERVE_P is set */
__thread int tls_preserve_used __attribute__((used)) = 42;

/* Variable without used attribute */
__thread int tls_preserve_normal = 100;

/* ===== DECL_CONTEXT testing ===== */
/* File scope TLS variable */
__thread int tls_file_scope = 1;

/* Static TLS variable (different context) */
static __thread int tls_static_scope = 2;

/* ===== TREE_USED testing ===== */
/* Variables that will be used in code */
__thread int tls_used_var1 = 10;
__thread int tls_used_var2 = 20;

/* Variable that might not be used (for comparison) */
__thread int tls_unused_var = 30;

/* ===== TREE_PUBLIC / DECL_EXTERNAL testing ===== */
/* Public TLS variable (non-static) */
__thread int tls_public = 50;

/* Static TLS variable (not public) */
static __thread int tls_non_public = 60;

/* ===== DECL_COMMON testing ===== */
/* Tentative definition - should create common symbol */
__thread int tls_common;

/* Initialized definition - not common */
__thread int tls_not_common = 70;

/* ===== DECL_WEAK testing ===== */
/* Weak TLS variable */
__thread int tls_weak_var __attribute__((weak)) = 80;

/* Strong TLS variable */
__thread int tls_strong_var = 90;

/* ===== DECL_VISIBILITY testing ===== */
/* Default visibility (explicit) */
__thread int tls_vis_default __attribute__((visibility("default"))) = 100;

/* Hidden visibility */
__thread int tls_vis_hidden __attribute__((visibility("hidden"))) = 200;

/* Protected visibility */
__thread int tls_vis_protected __attribute__((visibility("protected"))) = 300;

/* ===== DECL_VISIBILITY_SPECIFIED testing ===== */
/* Variable with explicitly specified visibility */
__thread int tls_vis_specified __attribute__((visibility("internal"))) = 400;

/* Variable without visibility attribute */
__thread int tls_vis_unspecified = 500;

/* ===== Function-scope TLS ===== */
void test_function_scope(void) {
    /* TLS variable with function scope context */
    static __thread int tls_func_scope = 600;
    tls_func_scope++;
}

/* ===== Complex usage patterns ===== */
/* Take address of TLS variable */
__thread int tls_address_taken = 700;

/* Use in inline assembly (forces preservation) */
__thread uint64_t tls_asm_used = 800;

/* ===== Main function ===== */
int main(void) {
    int result = 0;
    
    /* Ensure TREE_USED is set for these variables */
    result += tls_used_var1;
    result += tls_used_var2;
    
    /* Use all TLS variables to ensure they're marked as used */
    result += tls_preserve_used;
    result += tls_preserve_normal;
    result += tls_file_scope;
    result += tls_static_scope;
    result += tls_public;
    result += tls_non_public;
    
    /* Initialize common variable */
    tls_common = 150;
    result += tls_common;
    result += tls_not_common;
    
    /* Use weak and strong variables */
    result += tls_weak_var;
    result += tls_strong_var;
    
    /* Use visibility-controlled variables */
    result += tls_vis_default;
    result += tls_vis_hidden;
    result += tls_vis_protected;
    result += tls_vis_specified;
    result += tls_vis_unspecified;
    
    /* Test function scope TLS */
    test_function_scope();
    
    /* Take address and use */
    int *ptr = &tls_address_taken;
    *ptr = 750;
    result += *ptr;
    
    /* Use in inline assembly */
    uint64_t asm_val;
    __asm__ volatile (
        "movq %1, %0\n\t"
        : "=r" (asm_val)
        : "m" (tls_asm_used)
    );
    result += (int)asm_val;
    
    /* Force use of all variables through volatile to prevent optimization */
    volatile int force_use = 
        tls_preserve_used + tls_preserve_normal +
        tls_file_scope + tls_static_scope +
        tls_used_var1 + tls_used_var2 +
        tls_public + tls_non_public +
        tls_common + tls_not_common +
        tls_weak_var + tls_strong_var +
        tls_vis_default + tls_vis_hidden +
        tls_vis_protected + tls_vis_specified +
        tls_vis_unspecified + tls_address_taken;
    
    (void)force_use; /* Suppress unused variable warning */
    
    printf("Result: %d\n", result);
    return 0;
}
