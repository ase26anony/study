/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

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
/* Simulate similar attribute on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 500;

/* Additional patterns for completeness */
/* Common TLS variable (tentative definition) */
__thread double tls_common;

/* Public TLS with explicit visibility */
__thread float tls_public_default __attribute__((visibility("default")));

/* Noinline helper to force address materialization */
__attribute__((noinline)) 
static void use_tls_addresses(
    int *a, char *b, int *c, long *d, long *e, int *f, double *g, float *h)
{
    /* Dummy writes to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 1.0;
    if (h) *h += 1.0f;
    
    /* Prevent tail call optimization */
    volatile int dummy = 0;
    (void)dummy;
}

/* Another helper that returns values to prevent dead code elimination */
__attribute__((noinline))
static int compute_checksum(
    int a, char b, int c, long d, long e, int f, double g, float h)
{
    volatile int sum = 0;
    sum += a;
    sum += b;
    sum += c;
    sum += (int)(d & 0xFFFFFFFF);
    sum += (int)(e & 0xFFFFFFFF);
    sum += f;
    sum += (int)g;
    sum += (int)h;
    return sum;
}

int main(int argc, char **argv)
{
    int result = 0;
    
    /* Use argc to make behavior variable and prevent optimization */
    int use_argc = (argc > 0) ? argc : 1;
    
    /* Pattern A: Modify static initialized TLS */
    tls_static_init = use_argc * 10;
    
    /* Pattern B: Modify extern TLS */
    tls_extern = 'A' + (use_argc % 26);
    
    /* Pattern C: Modify weak TLS */
    tls_weak = use_argc * 20;
    
    /* Pattern D: Modify visibility TLS variables */
    tls_hidden = 1000 + use_argc;
    tls_protected = 2000 + use_argc * 2;
    
    /* Pattern E: Modify DLL-import-like TLS */
    tls_imported = 500 + use_argc * 5;
    
    /* Additional patterns */
    tls_common = 3.14159 * use_argc;
    tls_public_default = 2.71828f * use_argc;
    
    /* Force taking addresses of all TLS variables */
    use_tls_addresses(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_common,
        &tls_public_default
    );
    
    /* Use values in computation to prevent elimination */
    result = compute_checksum(
        tls_static_init,
        tls_extern,
        tls_weak,
        tls_hidden,
        tls_protected,
        tls_imported,
        tls_common,
        tls_public_default
    );
    
    /* Print result to ensure side effects */
    printf("TLS checksum: %d\n", result);
    
    /* Additional complex usage in loops */
    for (int i = 0; i < 3; i++) {
        tls_static_init += i;
        tls_extern += i;
        tls_hidden -= i;
    }
    
    /* Conditional access based on argc */
    if (argc > 1) {
        tls_protected = argv[1][0];
    }
    
    /* Return value depends on TLS variables */
    return (result + tls_static_init + tls_extern) % 256;
}

/* Force generation of TLS initialization code */
/* External reference to ensure TLS vars are emitted */
extern void *__dummy_refs[] = {
    (void*)&tls_static_init,
    (void*)&tls_extern,
    (void*)&tls_weak,
    (void*)&tls_hidden,
    (void*)&tls_protected,
    (void*)&tls_imported,
    (void*)&tls_common,
    (void*)&tls_public_default
};
