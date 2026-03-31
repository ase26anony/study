/* test_emutls.c - Comprehensive TLS emulation test */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls */

#include <stdio.h>
#include <stdint.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
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
__thread int tls_imported = 5000;
#else
/* Simulate similar behavior on non-Windows */
__thread int tls_imported __attribute__((weak)) = 5000;
#endif

/* Pattern F: Common TLS variable (tentative definition) */
__thread int tls_common;  /* Should become DECL_COMMON */

/* Pattern G: Used TLS variable with preserve attribute */
__thread volatile int tls_used_preserved __attribute__((used)) = 999;

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline, noipa))
static void use_tls_pointers(
    int *a, char *b, int *c, long *d, long *e, int *f, int *g, int *h)
{
    /* Dummy operations to prevent optimization */
    if (a) *a ^= 1;
    if (b) *b ^= 1;
    if (c) *c ^= 1;
    if (d) *d ^= 1;
    if (e) *e ^= 1;
    if (f) *f ^= 1;
    if (g) *g ^= 1;
    if (h) *h ^= 1;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Another helper to create control flow dependencies */
__attribute__((noinline))
static int compute_checksum(int argc, char **argv)
{
    int sum = 0;
    
    /* Use all TLS variables in computation */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 256;
    sum += tls_protected % 256;
    sum += tls_imported % 256;
    sum += tls_common;
    sum += tls_used_preserved % 256;
    
    /* Create argc-dependent behavior */
    if (argc > 1) {
        tls_static_init = argv[1][0];
        tls_extern = argv[1][0] ^ 1;
    }
    
    return sum;
}

int main(int argc, char **argv)
{
    int result = 0;
    
    /* Initialize/Modify TLS variables */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = (long)argc * 30;
    tls_protected = (long)argc * 40;
    tls_imported = argc * 50;
    tls_common = argc * 60;
    tls_used_preserved = argc * 70;
    
    /* Force taking addresses of all TLS variables */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_common,
        &tls_used_preserved
    );
    
    /* Use TLS variables in computations with control flow */
    for (int i = 0; i < argc; i++) {
        tls_common += i;
        if (i % 2 == 0) {
            tls_hidden += i;
        } else {
            tls_protected += i;
        }
    }
    
    /* Compute final result using all TLS variables */
    result = compute_checksum(argc, argv);
    
    /* Create observable output to prevent dead code elimination */
    printf("TLS test result: %d\n", result);
    printf("Values: %d, %c, %d, %ld, %ld, %d, %d, %d\n",
           tls_static_init, tls_extern, tls_weak,
           tls_hidden, tls_protected, tls_imported,
           tls_common, tls_used_preserved);
    
    return result & 0xFF;  /* Non-zero return based on TLS values */
}

/* Additional file to test external linkage (compile separately if needed) */
#ifdef MULTI_FILE_TEST
/* test_emutls2.c */
extern __thread int tls_extern_ref;
__thread int tls_extern_ref = 12345;

void dummy_func(void) {
    tls_extern_ref = 54321;
}
#endif
