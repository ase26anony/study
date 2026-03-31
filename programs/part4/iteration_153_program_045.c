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
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* On non-Windows, simulate with weak external */
extern __thread int tls_imported __attribute__((weak));
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different types and attributes */
__thread volatile float tls_volatile = 3.14f;
static __thread double tls_static_uninit;
__thread uint64_t tls_large __attribute__((aligned(64))) = 0xDEADBEEF;

/* Noinline helper to force address taking and prevent optimization */
__attribute__((noinline,noipa))
static void use_tls_pointers(
    int *a, char *b, int *c, long *d, long *e, 
    int *f, float *g, double *h, uint64_t *i)
{
    /* Dummy operations to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 0.5f;
    if (h) *h += 1.0;
    if (i) *i += 1;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Another noinline function that uses TLS values directly */
__attribute__((noinline,noipa))
static int compute_with_tls(int argc)
{
    int sum = 0;
    
    /* Use all TLS variables in computation */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_imported;
    sum += (int)tls_volatile;
    sum += (int)tls_static_uninit;
    sum += (int)(tls_large & 0xFF);
    
    /* Make computation depend on argc */
    return sum * (argc > 0 ? argc : 1);
}

/* Function that modifies TLS variables */
__attribute__((noinline))
static void modify_tls_variables(int argc)
{
    /* Modify each TLS variable based on argc */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    tls_imported = argc * 50;
    tls_volatile = argc * 3.14f;
    tls_static_uninit = argc * 2.71828;
    tls_large = (uint64_t)argc * 1000;
}

int main(int argc, char **argv)
{
    int result = 0;
    
    /* Initial computation */
    result = compute_with_tls(argc);
    
    /* Modify TLS variables */
    modify_tls_variables(argc);
    
    /* Take addresses of all TLS variables and pass to helper */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_volatile,
        &tls_static_uninit,
        &tls_large
    );
    
    /* Final computation */
    result += compute_with_tls(argc);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000000) {
        printf("Unexpected large result: %d\n", result);
        return 1;
    }
    
    /* Print something to ensure execution */
    printf("TLS test completed. Result: %d\n", result);
    
    return 0;
}
