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

/* Additional TLS variables with different types and storage */
__thread double tls_double = 3.14159;
__thread void* tls_pointer = NULL;

/* Noinline helper to force address materialization */
__attribute__((noinline, used))
static void use_tls_addresses(int *a, char *b, int *c, long *d, long *e, int *f, double *g, void **h) {
    /* Dummy operations to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 0.1;
    if (h) *h = (void*)((uintptr_t)*h + 1);
}

/* Another noinline function to use TLS values */
__attribute__((noinline, used))
static int compute_with_tls(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computations */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_imported;
    sum += (int)tls_double;
    sum += (uintptr_t)tls_pointer;
    
    /* Make result depend on argc */
    return sum * (argc + 1);
}

/* Function to test tentative definitions (for DECL_COMMON) */
__thread int tls_tentative;  /* Tentative definition - tests DECL_COMMON */

int main(int argc, char **argv) {
    int result = 0;
    
    /* Pattern A usage */
    tls_static_init = argc * 10;
    
    /* Pattern B usage */
    tls_extern = 'A' + (argc % 26);
    
    /* Pattern C usage - conditional based on weak symbol */
    if (&tls_weak) {
        tls_weak = argc * 20;
    }
    
    /* Pattern D usage */
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    
    /* Pattern E usage */
    tls_imported = argc * 50;
    
    /* Additional TLS variables */
    tls_double = argc * 3.14;
    tls_pointer = (void*)(uintptr_t)argc;
    
    /* Tentative definition usage */
    tls_tentative = argc * 60;
    
    /* Force address materialization for all TLS variables */
    use_tls_addresses(&tls_static_init,
                     &tls_extern,
                     &tls_weak,
                     &tls_hidden,
                     &tls_protected,
                     &tls_imported,
                     &tls_double,
                     &tls_pointer);
    
    /* Use TLS values in computation */
    result = compute_with_tls(argc);
    
    /* Add tentative definition to result */
    result += tls_tentative;
    
    /* Use volatile to prevent optimization */
    volatile int final_result = result;
    
    /* Print to prevent dead code elimination */
    printf("TLS test result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
/* This creates another reference to test external linkage */
__thread int tls_another_weak __attribute__((weak));

/* Function in different "compilation unit" (same file for simplicity) */
__attribute__((noinline, used))
static void use_weak_tls(void) {
    if (&tls_another_weak) {
        tls_another_weak = 999;
    }
}
