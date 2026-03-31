/* test_emutls.c - Test TLS emulation attribute copying */
#include <stdio.h>

/* Pattern A: Static, initialized TLS */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: Visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 200;
__thread long tls_protected __attribute__((visibility("protected"))) = 300;

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 400;
#else
/* Fallback for non-Windows: just a regular TLS variable */
__thread int tls_imported = 400;
#endif

/* Additional patterns for comprehensive coverage */
/* Common TLS (tentative definition) */
__thread double tls_common;

/* Used TLS with volatile to prevent optimization */
volatile __thread int tls_volatile = 500;

/* Noinline helper to force address materialization */
__attribute__((noinline)) 
static void use_tls_addresses(int *a, char *b, int *c, long *d, long *e, int *f, double *g, int *h) {
    /* Dummy writes to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 1.0;
    if (h) *h += 1;
}

/* Another helper that returns values to create data dependencies */
__attribute__((noinline))
static int compute_sum(int a, char b, int c, long d, long e, int f, double g, int h) {
    return a + b + c + d + e + f + (int)g + h;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* 1. Use all TLS variables with argc-dependent values */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    tls_imported = argc * 50;
    tls_common = argc * 3.14;
    tls_volatile = argc * 60;
    
    /* 2. Take addresses and pass to noinline function */
    use_tls_addresses(&tls_static_init, 
                     &tls_extern, 
                     &tls_weak, 
                     &tls_hidden, 
                     &tls_protected, 
                     &tls_imported, 
                     &tls_common, 
                     (int*)&tls_volatile);
    
    /* 3. Use values in computation with control flow */
    for (int i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_weak -= i;
    }
    
    /* 4. Compute final result using all TLS variables */
    result = compute_sum(tls_static_init,
                        tls_extern,
                        tls_weak,
                        tls_hidden,
                        tls_protected,
                        tls_imported,
                        tls_common,
                        tls_volatile);
    
    /* 5. Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result % 256;
}

/* Additional file-like separation using weak symbols */
/* Weak external reference to test DECL_EXTERNAL */
extern __thread int tls_external_ref __attribute__((weak));

/* Function in another "compilation unit" using TLS */
static void another_function(void) {
    static __thread int local_tls = 999;
    local_tls++;
    
    /* Reference weak external */
    if (&tls_external_ref) {
        tls_external_ref = local_tls;
    }
}

/* Call it to ensure it's not optimized away */
__attribute__((constructor))
static void init_tls(void) {
    another_function();
}
