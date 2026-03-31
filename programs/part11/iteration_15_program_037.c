/* Test file for emulated TLS attribute copying coverage */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Basic TLS variables with different attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC - non-static at file scope */
__thread int tls_public = 100;

/* Static TLS - not public */
static __thread int tls_static = 200;

/* DECL_COMMON - tentative definition */
__thread int tls_common;  /* No initializer at file scope */

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED - hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED - protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* DECL_VISIBILITY_SPECIFIED - internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* Pattern C: Complex usage to ensure processing */
__thread int* tls_pointer;

/* Function to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    tls_used += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common = tls_used + tls_public;
    tls_weak += 4;
    tls_hidden += 5;
    tls_protected += 6;
    tls_internal += 7;
    
    /* Take address to force additional processing */
    tls_pointer = &tls_used;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : "+m" (tls_public));
}

/* Pattern D: Different TLS models */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 800;
#endif

/* DECL_CONTEXT testing - TLS in function scope */
void test_function_context(void) {
    static __thread int tls_in_function = 900;
    tls_in_function++;
}

/* Main function that uses all TLS variables */
int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    tls_common = 50;  /* Initialize tentative definition */
    
    use_tls_variables();
    test_function_context();
    
    /* Complex expressions with TLS variables */
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    
#ifdef __GNUC__
    sum += tls_global_dynamic;
#endif
    
    /* Use tls_pointer */
    sum += *tls_pointer;
    
    printf("TLS sum: %d\n", sum);
    
    /* Return non-zero if any TLS variable has wrong value */
    if (sum != (42+1 + 100+2 + 200+3 + (42+1)+(100+2) + 300+4 + 
                400+5 + 500+6 + 600+7 + 800 + (42+1))) {
        return 1;
    }
    
    return 0;
}
