/* test_emutls.c - Comprehensive TLS emulation test */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls */

#include <stdio.h>
#include <stdint.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
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

/* TLS variable with both weak and visibility */
__thread double tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 500.0;

/* Uninitialized static TLS */
static __thread unsigned int tls_static_uninit;

/* Thread-local pointer */
__thread void* tls_pointer = NULL;

/* Noinline helper to force address materialization */
__attribute__((noinline, used))
static void use_tls_addresses(int *a, char *b, int *c, long *d, long *e, int *f, double *g, unsigned int *h, void **i) {
    /* Dummy writes to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 1.0;
    if (h) *h += 1;
    if (i) *i = (void*)((uintptr_t)*i + 1);
    
    /* Prevent tail call optimization */
    volatile int barrier = 0;
    (void)barrier;
}

/* Another noinline helper that uses TLS values */
__attribute__((noinline, used))
static int compute_with_tls(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computation */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += (int)tls_hidden;
    sum += (int)tls_protected;
    sum += tls_imported;
    sum += (int)tls_weak_hidden;
    sum += tls_static_uninit;
    sum += (int)(uintptr_t)tls_pointer;
    
    /* Make result dependent on argc */
    return sum * argc;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Initialize uninitialized TLS variables */
    if (argc > 1) {
        tls_static_uninit = argc * 10;
        tls_pointer = (void*)(uintptr_t)argc;
    }
    
    /* Modify TLS variables based on argc */
    tls_static_init = argc;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 2;
    tls_hidden = argc * 3L;
    tls_protected = argc * 4L;
    tls_imported = argc * 5;
    tls_weak_hidden = argc * 6.0;
    
    /* Force address materialization of all TLS variables */
    use_tls_addresses(&tls_static_init,
                     &tls_extern,
                     &tls_weak,
                     &tls_hidden,
                     &tls_protected,
                     &tls_imported,
                     &tls_weak_hidden,
                     &tls_static_uninit,
                     &tls_pointer);
    
    /* Use TLS values in computation */
    result = compute_with_tls(argc);
    
    /* Create control flow dependencies on TLS values */
    volatile int control = 0;
    
    if (tls_static_init > 0) {
        control += 1;
        if (tls_extern == 'A') {
            control += 2;
        }
    }
    
    if (tls_hidden != tls_protected) {
        for (int i = 0; i < tls_weak % 10; i++) {
            control += i;
        }
    }
    
    /* Final result depends on everything */
    result += control;
    
    /* Print to prevent optimization */
    printf("Result: %d (TLS values: %d, %c, %d, %ld, %ld, %d, %.1f, %u, %p)\n",
           result,
           tls_static_init,
           tls_extern,
           tls_weak,
           tls_hidden,
           tls_protected,
           tls_imported,
           tls_weak_hidden,
           tls_static_uninit,
           tls_pointer);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}

/* Additional file-like separation using weak symbols */
/* This tests DECL_COMMON and DECL_EXTERNAL behaviors */

/* Weak external reference */
extern __thread int tls_tentative __attribute__((weak));

/* Definition that might be common */
__thread int tls_tentative;

/* Function that uses the tentative definition */
__attribute__((noinline))
static void use_tentative(void) {
    if (&tls_tentative) {
        tls_tentative = 999;
    }
}
