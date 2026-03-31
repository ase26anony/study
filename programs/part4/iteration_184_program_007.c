/* This should trigger emulated TLS code generation */

/* Force emulated TLS by using appropriate compilation flags:
   -femulated-tls or -ftls-model=emulated
   Target architecture: e.g., -march=armv7-a (without native TLS support)
*/

#include <stdio.h>

/* TLS variables with various attributes to test attribute copying */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static (internal linkage), initialized */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS variable - should set DECL_WEAK */
__attribute__((weak)) __thread int tls_weak = 5;

/* TLS with hidden visibility - tests DECL_VISIBILITY */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 6;

/* TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible_default = 7;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 8;

/* Uninitialized TLS variable */
__thread int tls_uninit;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* For non-Windows, we can't truly test dllimport, but declare it anyway */
__thread int tls_dllimport;
#endif

/* Definition of the extern TLS variable */
__thread int tls_extern = 3;

/* Another weak TLS variable, uninitialized */
__attribute__((weak)) __thread int tls_weak_uninit;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    if (tls_weak == 0) {
        tls_weak = 100;
    }
    
    tls_hidden = tls_visible_default + 1;
    tls_used = tls_default + tls_static;
    
    /* Ensure tls_uninit is used */
    tls_uninit = 42;
    
    /* Use dllimport variable if available */
    tls_dllimport = 99;
}

/* Another helper to take addresses of TLS variables */
void use_tls_pointers(void) {
    /* Taking addresses forces the compiler to generate TLS symbols */
    int *ptr1 = &tls_default;
    int *ptr2 = &tls_static;
    int *ptr3 = &tls_extern;
    int *ptr4 = &tls_weak;
    int *ptr5 = &tls_hidden;
    int *ptr6 = &tls_visible_default;
    int *ptr7 = &tls_used;
    int *ptr8 = &tls_uninit;
    
    /* Use pointers to create side effects */
    if (ptr1 != ptr2) {
        *ptr1 += 1;
    }
    
    /* Prevent optimization of unused pointers */
    (void)ptr3;
    (void)ptr4;
    (void)ptr5;
    (void)ptr6;
    (void)ptr7;
    (void)ptr8;
}

int main(void) {
    int sum = 0;
    
    /* Initialize uninitialized TLS variables */
    tls_uninit = 4;
    tls_weak_uninit = 9;
    
    /* Use all TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible_default;
    sum += tls_used;
    sum += tls_uninit;
    sum += tls_weak_uninit;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum */
    sum = tls_default + tls_static + tls_extern + tls_weak +
          tls_hidden + tls_visible_default + tls_used +
          tls_uninit + tls_weak_uninit;
    
    printf("Modified sum: %d\n", sum);
    
    /* Use TLS pointers */
    use_tls_pointers();
    
    /* Final calculation */
    sum = tls_default + tls_static + tls_extern + tls_weak +
          tls_hidden + tls_visible_default + tls_used +
          tls_uninit + tls_weak_uninit;
    
    printf("Final sum: %d\n", sum);
    
    return 0;
}
