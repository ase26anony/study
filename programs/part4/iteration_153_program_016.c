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

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different types and attributes */
__thread volatile double tls_volatile = 3.14159;
static __thread unsigned tls_static_uninit;

/* Noinline helper to force address materialization */
__attribute__((noinline)) 
static void use_tls_addresses(int *a, char *b, int *c, long *d, long *e, int *f, double *g, unsigned *h) {
    /* Dummy operations to prevent optimization */
    if (a) *a ^= 1;
    if (b) *b += 1;
    if (c) *c |= 0x01;
    if (d) *d += 100;
    if (e) *e += 200;
    if (f) *f += 300;
    if (g) *g += 0.5;
    if (h) *h = 1;
}

/* Another noinline function to use TLS values */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    result += tls_static_init * argc;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 256;
    result += tls_protected % 256;
    result += tls_imported % 256;
    result += (int)tls_volatile;
    result += tls_static_uninit;
    
    return result;
}

int main(int argc, char **argv) {
    int i, sum = 0;
    
    /* Force initialization and usage of all TLS variables */
    if (argc > 1) {
        tls_static_init = argc * 10;
        tls_extern = 'A' + (argc % 26);
        tls_weak = argc * 20;
        tls_hidden = argc * 100L;
        tls_protected = argc * 200L;
        tls_imported = argc * 50;
        tls_volatile = argc * 3.14;
        tls_static_uninit = argc * 2;
    }
    
    /* Loop to create control flow dependencies */
    for (i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_extern = (tls_extern + 1) & 0x7F;
        tls_weak ^= i;
    }
    
    /* Take addresses of all TLS variables */
    use_tls_addresses(&tls_static_init,
                     &tls_extern,
                     &tls_weak,
                     &tls_hidden,
                     &tls_protected,
                     &tls_imported,
                     &tls_volatile,
                     &tls_static_uninit);
    
    /* Compute using TLS values */
    sum = compute_with_tls(argc);
    
    /* Use result to prevent dead code elimination */
    printf("TLS computation result: %d\n", sum);
    
    /* Additional complex usage pattern */
    {
        __thread int local_tls = sum;
        __thread static int static_local_tls = 99;
        
        local_tls += argc;
        static_local_tls += sum % 100;
        
        use_tls_addresses(&local_tls, NULL, &static_local_tls, NULL, NULL, NULL, NULL, NULL);
        
        printf("Local TLS values: %d, %d\n", local_tls, static_local_tls);
    }
    
    return sum > 100 ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
/* This tests DECL_COMMON and linkage behavior */
__thread int tls_common __attribute__((weak));
__thread int tls_common = 9999;

/* Function that references the weak TLS */
void use_weak_tls(void) {
    tls_common++;
}
