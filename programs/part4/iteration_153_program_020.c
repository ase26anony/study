/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern/public TLS variable with tentative definition */
extern __thread char tls_extern;
__thread char tls_extern = 'X';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
#else
/* Simulate with weak external on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif

/* Provide definition for tls_imported */
__thread int tls_imported = 999;

/* Additional TLS variable with complex type */
typedef struct {
    int a;
    double b;
    char c[4];
} ComplexTLS;
__thread ComplexTLS tls_complex = {1, 3.14, "abc"};

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline)) 
static void use_tls_pointers(
    int *p1, char *p2, int *p3, long *p4, long *p5, 
    int *p6, ComplexTLS *p7, volatile int *dummy) 
{
    /* Dummy operations to prevent optimization */
    if (p1) *dummy += *p1;
    if (p2) *dummy += *p2;
    if (p3) *dummy += *p3;
    if (p4) *dummy += *p4;
    if (p5) *dummy += *p5;
    if (p6) *dummy += *p6;
    if (p7) *dummy += p7->a;
}

/* Another helper that returns different values based on TLS state */
__attribute__((noinline))
static int compute_from_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computation */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 256;
    result += tls_protected % 256;
    result += tls_imported;
    result += tls_complex.a;
    
    /* Make result depend on argc for variability */
    return result * (argc > 0 ? argc : 1);
}

int main(int argc, char **argv) {
    volatile int dummy = 0;
    
    /* 1. Modify TLS variables based on program input */
    if (argc > 1) {
        tls_static_init = argv[1][0];
        tls_extern = (char)(argv[1][0] + 1);
        tls_weak = argc * 10;
        tls_hidden = argc * 100L;
        tls_protected = argc * 200L;
        tls_imported = argc * 50;
        tls_complex.a = argc;
    }
    
    /* 2. Take addresses of all TLS variables */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_complex,
        &dummy
    );
    
    /* 3. Use TLS values in control flow */
    int sum = 0;
    for (int i = 0; i < (tls_static_init & 0x3); i++) {
        sum += tls_extern;
        sum += tls_weak >> i;
    }
    
    /* 4. Compute final result using helper */
    int result = compute_from_tls(argc) + sum;
    
    /* 5. Print to prevent optimization */
    printf("TLS test result: %d (dummy=%d)\n", result, dummy);
    
    return (result > 100) ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
/* This tests DECL_COMMON and linkage behavior */
__thread int tls_common1;
extern __thread int tls_common2;
__thread int tls_common2;  /* Tentative definition */

/* Function in separate compilation unit simulation */
static void use_common_tls(void) {
    tls_common1 = 123;
    tls_common2 = 456;
    volatile int *p1 = &tls_common1;
    volatile int *p2 = &tls_common2;
    (void)p1;
    (void)p2;
}
