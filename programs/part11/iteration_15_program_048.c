/* tls_main.c - Main test file for emulated TLS attribute coverage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS for all variables */
#pragma GCC tls_model emulated

/* DECL_PRESERVE_P: Variables that should not be eliminated */
__thread int tls_preserve __attribute__((used)) = 42;
__thread int tls_not_preserve = 100;

/* TREE_USED: Variables that will be referenced */
__thread int tls_used1 = 1;
__thread int tls_used2 = 2;
static __thread int tls_static_used = 3;

/* TREE_PUBLIC: Mix of public and static */
__thread int tls_public = 10;
static __thread int tls_static = 20;

/* DECL_COMMON: Tentative definitions (common symbols) */
__thread int tls_common;  /* No initializer = common symbol */
__thread int tls_common2;

/* DECL_WEAK: Weak TLS variables */
__thread int tls_weak __attribute__((weak)) = 30;
extern __thread int tls_weak_extern __attribute__((weak));

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_default __attribute__((visibility("default"))) = 40;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 50;
__thread int tls_protected __attribute__((visibility("protected"))) = 60;
#ifdef __GNUC__
__thread int tls_internal __attribute__((visibility("internal"))) = 70;
#endif

/* DECL_EXTERNAL: External declaration */
extern __thread int tls_external;

/* Function to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    tls_used1 += 1;
    tls_used2 += 2;
    tls_static_used += 3;
    tls_public += tls_used1;
    tls_static += tls_used2;
    
    /* Use in complex expression to ensure processing */
    int* ptr = &tls_preserve;
    *ptr += tls_not_preserve;
    
    /* Take address to prevent optimizations */
    volatile int* volatile_ptr = &tls_common;
    (void)volatile_ptr;
}

/* DECL_CONTEXT: TLS variable in function scope */
void test_function_context(void) {
    static __thread int tls_in_function = 99;
    tls_in_function++;
    
    /* Use in inline assembly to ensure processing */
    __asm__ volatile ("" : "+m" (tls_in_function));
}

/* Force emulated TLS lowering with specific model */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 80;
__thread int tls_emulated __attribute__((tls_model("emulated"))) = 90;

int main(void) {
    int sum = 0;
    
    /* Initialize common symbols */
    tls_common = 1000;
    tls_common2 = 2000;
    
    /* Use all TLS variables */
    use_tls_variables();
    test_function_context();
    
    /* Reference weak variables */
    if (&tls_weak) {
        sum += tls_weak;
    }
    
    /* Reference external variable (defined in another file) */
    sum += tls_external;
    
    /* Reference all visibility variables */
    sum += tls_default;
    sum += tls_hidden;
    sum += tls_protected;
#ifdef __GNUC__
    sum += tls_internal;
#endif
    
    /* Reference model-specific variables */
    sum += tls_global_dynamic;
    sum += tls_emulated;
    
    /* Reference preserve variables */
    sum += tls_preserve;
    sum += tls_not_preserve;
    
    printf("TLS test sum: %d\n", sum);
    
    /* Ensure all variables are truly used by returning their sum */
    return sum > 0 ? 0 : 1;
}
