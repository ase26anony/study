/* test_emutls_attributes.c
 * This should trigger emulated TLS code generation
 * Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC
 * Or: -O2 -march=armv7-a -ftls-model=emulated
 */

#include <stdio.h>

/* Force declaration attributes to be set on TLS variables */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static (internal) linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak symbol with explicit visibility */
__attribute__((weak)) __attribute__((visibility("default"))) 
__thread int tls_weak = 5;

/* Hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use a different attribute that might set similar flags */
__attribute__((weak)) __thread int tls_dllimport = 8;
#endif

/* Common symbol (uninitialized external) */
__thread int tls_common;

/* Definition of previously declared extern */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden = tls_default + tls_static;
    
    /* Take address to inhibit optimizations */
    int *ptr = &tls_weak;
    *ptr += 1;
    
    /* Use the used variable */
    tls_used++;
}

/* Another helper that takes TLS address */
void use_tls_address(int **addr) {
    static int counter = 0;
    *addr = &tls_extern;
    tls_extern = counter++;
}

int main(void) {
    int result = 0;
    int *tls_ptr;
    
    /* Initialize some variables */
    tls_hidden = 4;
    tls_common = 6;
    
    /* Use all TLS variables to prevent elimination */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    result += tls_hidden;
    result += tls_weak;
    result += tls_used;
    result += tls_common;
    
#ifdef _WIN32
    /* On Windows, dllimport is just a declaration */
    extern __thread int tls_dllimport;
    result += 1;  /* Compensate for missing variable */
#else
    result += tls_dllimport;
#endif
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Take address of TLS variable */
    use_tls_address(&tls_ptr);
    *tls_ptr = 100;
    
    /* Recalculate with modified values */
    result = tls_default + tls_static + tls_extern + 
             tls_hidden + tls_weak + tls_used + tls_common;
    
#ifdef _WIN32
    result += 1;
#else
    result += tls_dllimport;
#endif
    
    printf("Modified sum: %d\n", result);
    
    /* Create side effect with TLS address */
    if (tls_ptr == &tls_extern) {
        printf("TLS address correctly taken\n");
    }
    
    return 0;
}
