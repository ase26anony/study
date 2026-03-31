/* Test case for TLS emulation attribute copying in tree-emutls.cc
 * This should trigger emulated TLS code generation
 */

#include <stdio.h>

/* Force emulated TLS handling */
#ifdef __GNUC__
#define TLS_ATTR __thread
#else
#define TLS_ATTR _Thread_local
#endif

/* TLS variables with various attributes and linkages */

/* Default external linkage, initialized */
TLS_ATTR int tls_default = 1;

/* Static linkage */
static TLS_ATTR int tls_static = 2;

/* External declaration (simulating header) */
extern TLS_ATTR int tls_extern;

/* External definition */
TLS_ATTR int tls_extern = 3;

/* Weak symbol */
__attribute__((weak)) TLS_ATTR int tls_weak = 4;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) TLS_ATTR int tls_used = 5;

/* Hidden visibility */
__attribute__((visibility("hidden"))) TLS_ATTR int tls_hidden = 6;

/* Default visibility (explicit) */
__attribute__((visibility("default"))) TLS_ATTR int tls_visible = 7;

/* DLL import attribute (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) TLS_ATTR int tls_dllimport;
#else
/* Simulate dllimport for cross-compilation testing */
__attribute__((dllimport)) TLS_ATTR int tls_dllimport = 8;
#endif

/* Common linkage (uninitialized) */
TLS_ATTR int tls_common;

/* Weak and hidden */
__attribute__((weak, visibility("hidden"))) TLS_ATTR int tls_weak_hidden;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS if available */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Use hidden TLS */
    tls_hidden = tls_default + tls_static;
    
    /* Ensure used TLS is accessed */
    tls_used++;
    
    /* Take address to inhibit optimizations */
    volatile int *addr = &tls_visible;
    (void)addr;
}

/* Another function to ensure TLS variables are processed */
void check_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *ptrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_used,
        &tls_hidden,
        &tls_visible,
        &tls_common
    };
    
    /* Use the addresses to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < (int)(sizeof(ptrs)/sizeof(ptrs[0])); i++) {
        sum += *ptrs[i];
    }
    (void)sum;
}

int main(void) {
    /* Initialize uninitialized TLS */
    tls_common = 9;
    tls_weak_hidden = 10;
    
    /* Print initial values */
    printf("Initial TLS values:\n");
    printf("  tls_default: %d\n", tls_default);
    printf("  tls_static: %d\n", tls_static);
    printf("  tls_extern: %d\n", tls_extern);
    printf("  tls_weak: %d\n", tls_weak);
    printf("  tls_used: %d\n", tls_used);
    printf("  tls_hidden: %d\n", tls_hidden);
    printf("  tls_visible: %d\n", tls_visible);
    printf("  tls_common: %d\n", tls_common);
    printf("  tls_weak_hidden: %d\n", tls_weak_hidden);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Check addresses */
    check_tls_addresses();
    
    /* Compute and print result using TLS variables */
    int result = tls_default + tls_static + tls_extern + 
                 tls_used + tls_hidden + tls_visible + 
                 tls_common + tls_weak_hidden;
    
    printf("Result after modifications: %d\n", result);
    
    /* Take address of a TLS variable and use it */
    volatile int *tls_addr = &tls_default;
    if (tls_addr) {
        *tls_addr += 1;
    }
    
    printf("Final tls_default: %d\n", tls_default);
    
    return 0;
}
