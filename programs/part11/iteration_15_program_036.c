/* Test for emulated TLS attribute copying - C version */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Explicit emulated TLS with various attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC + DECL_COMMON - tentative definition */
__thread int tls_common;

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* DECL_VISIBILITY_SPECIFIED - hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* DECL_VISIBILITY_SPECIFIED - protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 300;

/* Static TLS (not TREE_PUBLIC) */
static __thread int tls_static = 500;

/* DECL_EXTERNAL - external declaration */
extern __thread int tls_external;

/* Function to ensure TREE_USED is set */
void use_tls_vars(void) {
    tls_used += 1;
    tls_common = tls_used * 2;
    tls_weak = tls_common + 1;
    tls_hidden = tls_weak * 2;
    tls_protected = tls_hidden + 1;
    tls_static = tls_protected * 2;
    
    /* Reference external */
    if (&tls_external != NULL) {
        /* Just taking address ensures it's used */
    }
}

/* DECL_CONTEXT - TLS in function scope */
void func_with_tls(void) {
    static __thread int tls_in_func = 600;
    tls_in_func++;
}

/* Complex usage to force processing */
int* get_tls_address(void) {
    return &tls_used;
}

/* Use in asm to ensure preservation */
void asm_use(void) {
    int val;
    __asm__ volatile (
        "movl %1, %0\n\t"
        : "=r"(val)
        : "m"(tls_used)
    );
}

int main(void) {
    use_tls_vars();
    func_with_tls();
    asm_use();
    
    /* Ensure all TLS vars are referenced */
    int sum = tls_used + tls_common + tls_weak + 
              tls_hidden + tls_protected + tls_static;
    
    printf("TLS sum: %d\n", sum);
    return 0;
}
