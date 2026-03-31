/* test_emutls.c - Comprehensive test for TLS emulation attribute copying */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static TLS with initialization */
static __thread int tls_static_init = 42;

/* Pattern B: External/public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 500;
#else
/* On non-Windows, use weak attribute to simulate similar linkage */
extern __thread int tls_imported __attribute__((weak));
__thread int tls_imported = 500;
#endif

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline)) 
static void use_tls_pointers(
    int *p1, char *p2, int *p3, 
    long *p4, long *p5, int *p6
) {
    /* Dummy writes to ensure addresses are used */
    if (p1) *p1 += 1;
    if (p2) *p2 += 1;
    if (p3) *p3 += 1;
    if (p4) *p4 += 1;
    if (p5) *p5 += 1;
    if (p6) *p6 += 1;
    
    /* Prevent tail-call optimization */
    volatile int dummy = 0;
    (void)dummy;
}

/* Another helper to create control flow dependencies */
__attribute__((noinline))
static int compute_checksum(int argc) {
    int sum = 0;
    
    /* Use TLS variables in computation with argc dependency */
    tls_static_init = argc * 2;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 3;
    tls_hidden = argc * 100L;
    tls_protected = argc * 200L;
    tls_imported = argc * 5;
    
    /* Create control flow that depends on TLS values */
    for (int i = 0; i < (tls_static_init % 10); i++) {
        sum += tls_weak + i;
    }
    
    if (tls_extern > 'M') {
        sum += tls_hidden;
    } else {
        sum += tls_protected;
    }
    
    sum += tls_imported;
    
    return sum;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Ensure all TLS variables are marked as used */
    TREE_USED(&tls_static_init);
    TREE_USED(&tls_extern);
    TREE_USED(&tls_weak);
    TREE_USED(&tls_hidden);
    TREE_USED(&tls_protected);
    TREE_USED(&tls_imported);
    
    /* Take addresses of all TLS variables */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported
    );
    
    /* Compute with TLS variables */
    result = compute_checksum(argc);
    
    /* Use TLS variables in output to prevent elimination */
    printf("TLS values: %d, %c, %d, %ld, %ld, %d\n",
           tls_static_init,
           tls_extern,
           tls_weak,
           tls_hidden,
           tls_protected,
           tls_imported);
    
    printf("Result: %d\n", result);
    
    /* Return value depends on TLS state */
    return (result > 1000) ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
/* This creates DECL_EXTERNAL and DECL_COMMON cases */
__thread int tls_tentative;  /* Tentative definition - tests DECL_COMMON */

/* Reference the tentative definition */
void reference_tentative(void) {
    tls_tentative = 123;
}
