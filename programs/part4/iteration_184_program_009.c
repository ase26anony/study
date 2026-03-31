/* Test case for TLS emulation attribute copying in tree-emutls.cc
 * This should trigger emulated TLS code generation
 * Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC
 */

#include <stdio.h>

/* Force emulated TLS attribute copying */
#ifdef __GNUC__
#define TLS_ATTR __thread
#else
#define TLS_ATTR _Thread_local
#endif

/* Various TLS declarations with different attributes */

/* Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS with internal linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Definition of external TLS */
__thread int tls_extern = 3;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 4;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible = 6;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* Common TLS (uninitialized) - may trigger DECL_COMMON */
__thread int tls_common;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_imported;
#else
/* Simulate similar attribute on non-Windows */
__attribute__((weak)) __thread int tls_imported = 8;
#endif

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden -= 1;
    tls_used = tls_default + tls_static;
    
    /* Take address to inhibit optimizations */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    
    /* Use addresses to create side effects */
    if (addr1 != addr2) {
        tls_common = *addr1 + *addr2;
    }
}

/* Another helper to ensure TLS variables are referenced */
int compute_tls_sum(void) {
    int sum = 0;
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_used;
    sum += tls_common;
    sum += tls_imported;
    
    return sum;
}

/* Function that takes TLS address as parameter */
void use_tls_pointer(int *ptr) {
    if (ptr) {
        *ptr += 1;
    }
}

int main(void) {
    /* Initialize uninitialized TLS */
    tls_common = 9;
    
    /* Use all TLS variables in main */
    printf("Initial TLS values:\n");
    printf("tls_default: %d\n", tls_default);
    printf("tls_static: %d\n", tls_static);
    printf("tls_extern: %d\n", tls_extern);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_visible: %d\n", tls_visible);
    printf("tls_used: %d\n", tls_used);
    printf("tls_common: %d\n", tls_common);
    printf("tls_imported: %d\n", tls_imported);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Take address of TLS variable */
    int *tls_ptr = &tls_default;
    use_tls_pointer(tls_ptr);
    
    /* Compute sum using TLS variables */
    int total = compute_tls_sum();
    printf("\nTLS sum after modifications: %d\n", total);
    
    /* Additional operations to ensure all code paths are used */
    tls_weak = total % 100;
    tls_visible = tls_hidden + tls_static;
    
    /* Final sum */
    int final_sum = compute_tls_sum();
    printf("Final TLS sum: %d\n", final_sum);
    
    return final_sum > 0 ? 0 : 1;
}

/* Additional TLS definition to test DECL_EXTERNAL handling */
__thread int another_tls __attribute__((weak)) = 100;

/* Function using the additional TLS */
void use_another_tls(void) {
    another_tls += tls_default;
}
