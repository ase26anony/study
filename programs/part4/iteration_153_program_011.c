/* test_emutls.c - Test program to cover TLS emulation attribute copying */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls_executable */
/* Or for 32-bit: gcc -m32 -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls_executable */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable with tentative definition */
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
/* On non-Windows, simulate with weak external */
extern __thread int tls_imported __attribute__((weak));
#endif

/* Definition for the imported variable */
__thread int tls_imported = 5000;

/* Additional TLS variable for more coverage */
__thread double tls_global = 3.14159;

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline)) 
static void use_tls_pointers(
    int *a, char *b, int *c, long *d, long *e, int *f, double *g) {
    /* Dummy writes to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 0.5;
    
    /* Force memory clobber to prevent reordering */
    __asm__ __volatile__("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g) : "memory");
}

/* Another helper to use TLS values in computation */
__attribute__((noinline))
static int compute_checksum(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computation */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 100;
    sum += tls_protected % 100;
    sum += tls_imported % 100;
    sum += (int)tls_global;
    
    /* Make result dependent on argc */
    return sum * (argc + 1);
}

int main(int argc, char **argv) {
    int result;
    
    /* Step 1: Assign values based on argc to create variability */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 100L;
    tls_protected = argc * 200L;
    tls_imported = argc * 50;
    tls_global = argc * 1.5;
    
    /* Step 2: Use TLS variables in conditional logic */
    if (argc > 1) {
        tls_static_init += 100;
        tls_extern = 'Z';
    }
    
    /* Step 3: Loop using TLS variables */
    for (int i = 0; i < (argc % 5); i++) {
        tls_weak += i;
        tls_global += 0.1;
    }
    
    /* Step 4: Take addresses and pass to helper function */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_global
    );
    
    /* Step 5: Compute final result using all TLS variables */
    result = compute_checksum(argc);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (argc=%d)\n", result, argc);
    
    /* Additional volatile access to ensure TLS is referenced */
    volatile int dummy = tls_static_init + tls_weak;
    (void)dummy;  /* Suppress unused warning */
    
    return result % 256;
}

/* Additional file-scope TLS to test more cases */
__thread int tls_file_scope = 999;
static __thread float tls_static_uninit;

/* Function in another compilation unit if split compilation is used */
#ifdef SPLIT_COMPILATION
/* In a separate file, you would have: */
/* extern __thread int tls_extern; */
/* __thread int tls_extern = 123; */
#endif
