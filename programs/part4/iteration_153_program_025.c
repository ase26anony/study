/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
#else
/* Simulate similar behavior with weak attribute */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 5000;

/* Additional patterns for comprehensive coverage */
/* TLS variable with both weak and visibility */
__thread double tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 3.14;

/* Common (tentative definition) pattern */
__thread short tls_common;  /* Tentative definition */

/* Volatile TLS to prevent optimizations */
volatile __thread int tls_volatile = 99;

/* Noinline helper function to force address taking */
__attribute__((noinline)) 
static void use_tls_addresses(int *a, char *b, int *c, long *d, long *e, int *f, double *g, short *h, int *i) {
    /* Dummy writes to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 0.1;
    if (h) *h += 1;
    if (i) *i += 1;
}

/* Another noinline function to use TLS values */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computation */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_imported;
    sum += (int)tls_weak_hidden;
    sum += tls_common;
    sum += tls_volatile;
    
    /* Make computation depend on argc */
    return sum * (argc > 0 ? argc : 1);
}

int main(int argc, char **argv) {
    int result;
    
    /* Initialize TLS variables with argc-dependent values */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    tls_imported = argc * 50;
    tls_weak_hidden = argc * 3.14159;
    tls_common = argc * 2;
    tls_volatile = argc * 5;
    
    /* Force taking addresses of all TLS variables */
    use_tls_addresses(&tls_static_init,
                     &tls_extern,
                     &tls_weak,
                     &tls_hidden,
                     &tls_protected,
                     &tls_imported,
                     &tls_weak_hidden,
                     &tls_common,
                     (int*)&tls_volatile);
    
    /* Use TLS values in computation with control flow */
    result = compute_with_tls(argc);
    
    /* Complex control flow that depends on TLS values */
    if (tls_static_init > 100) {
        tls_hidden *= 2;
    }
    
    for (int i = 0; i < (tls_weak % 10); i++) {
        tls_protected += i;
    }
    
    /* Switch based on TLS value */
    switch (tls_extern % 4) {
        case 0: tls_imported += 100; break;
        case 1: tls_imported += 200; break;
        case 2: tls_imported += 300; break;
        default: tls_imported += 400; break;
    }
    
    /* Final computation using all modified values */
    result += tls_static_init + tls_extern + tls_weak + 
              tls_hidden + tls_protected + tls_imported + 
              (int)tls_weak_hidden + tls_common + tls_volatile;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d (argc=%d)\n", result, argc);
    
    return result > 1000 ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
/* This creates another "file scope" for testing DECL_COMMON and DECL_EXTERNAL */
__attribute__((weak))
__thread int tls_another_weak;

void init_weak_tls(void) {
    tls_another_weak = 999;
}
