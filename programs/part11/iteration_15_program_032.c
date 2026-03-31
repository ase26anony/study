/* Main file with various TLS declarations */
#include <stdio.h>

/* Pattern A: Explicit emulated TLS model */
__thread int tls_emulated __attribute__((used)) = 42;  /* DECL_PRESERVE_P via used */

/* Pattern B: Public TLS with initialization */
__thread int tls_public = 100;  /* TREE_PUBLIC, non-static */

/* Pattern C: Static TLS (not public) */
static __thread int tls_static = 200;  /* Not TREE_PUBLIC */

/* Pattern D: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;  /* DECL_WEAK */

/* Pattern E: Hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;  /* DECL_VISIBILITY */

/* Pattern F: Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* Pattern G: Common symbol (tentative definition) */
__thread int tls_common;  /* DECL_COMMON, no initializer */

/* Pattern H: Used in complex expression */
__thread int* tls_ptr;  /* Will take address */

/* External declaration from another file */
extern __thread int tls_extern;

/* Function to ensure TREE_USED is set */
void use_tls_vars(void) {
    tls_emulated += 1;
    tls_public += 2;
    tls_static += 3;
    tls_weak += 4;
    tls_hidden += 5;
    tls_protected += 6;
    tls_common = 700;  /* Initialize common */
    tls_ptr = &tls_emulated;
    
    /* Use extern */
    if (&tls_extern) {
        tls_extern = 800;
    }
}

/* Complex usage to prevent optimization */
int complex_tls_usage(void) {
    int sum = tls_emulated;
    sum += tls_public;
    sum += tls_static;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_common;
    
    /* Address-taking and pointer dereference */
    __thread int local_tls __attribute__((used)) = 900;  /* DECL_CONTEXT in function */
    sum += local_tls;
    
    /* Inline asm to ensure preservation */
    __asm__ volatile ("" : : "r"(&local_tls));
    
    return sum;
}

int main(void) {
    use_tls_vars();
    int result = complex_tls_usage();
    
    /* Print to prevent dead code elimination */
    printf("TLS result: %d\n", result);
    printf("Addresses: %p %p %p\n", 
           (void*)&tls_emulated,
           (void*)&tls_public,
           (void*)&tls_static);
    
    return 0;
}
