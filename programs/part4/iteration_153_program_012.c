/* test_emutls.c - Comprehensive TLS emulation test */

/* Prevent inlining to ensure addresses are taken */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern A: Static, initialized TLS */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows: just a regular TLS with different name */
__thread int tls_regular = 5000;
#endif

/* Additional TLS variables with different types and storage */
__thread volatile double tls_volatile = 3.14159;
static __thread unsigned long tls_static_uninit;

/* Helper function to force address usage */
NOINLINE static void use_tls_addresses(
    int *a, char *b, int *c, long *d, long *e, 
#ifdef __MINGW32__
    int *f,
#else
    int *f,
#endif
    double *g, unsigned long *h)
{
    /* Dummy writes to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 0.1;
    if (h) *h = (unsigned long)g;
    
    /* Memory barrier to prevent reordering */
    __asm__ __volatile__("" : : : "memory");
}

/* Another helper to create control flow dependencies */
NOINLINE static int compute_checksum(int argc)
{
    int sum = 0;
    
    /* Use all TLS variables in computation */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 100;
    sum += tls_protected % 100;
    
#ifdef __MINGW32__
    sum += tls_imported % 100;
#else
    sum += tls_regular % 100;
#endif
    
    sum += (int)tls_volatile;
    sum += tls_static_uninit % 100;
    
    /* Create argc-dependent control flow */
    if (argc > 1) {
        tls_static_init *= 2;
        tls_extern = 'C';
        tls_weak += argc;
    } else {
        tls_hidden -= argc;
        tls_protected += argc;
    }
    
    /* Loop with TLS dependency */
    for (int i = 0; i < argc % 5; i++) {
        tls_static_uninit += i;
    }
    
    return sum;
}

/* Function to test external linkage simulation */
NOINLINE static void test_external_linkage(void)
{
    /* Tentative definition to test DECL_COMMON */
    __thread int tls_tentative;
    tls_tentative = 999;
    
    /* Use it to prevent optimization */
    tls_static_init += tls_tentative % 10;
}

int main(int argc, char **argv)
{
    int result = 0;
    
    /* Initialize/Modify TLS variables */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 100;
    tls_hidden = 1234 + argc;
    tls_protected = 5678 + argc;
    
#ifdef __MINGW32__
    tls_imported = 9999 + argc;
#else
    tls_regular = 9999 + argc;
#endif
    
    tls_volatile = (double)argc / 2.0;
    tls_static_uninit = (unsigned long)argc * 1000;
    
    /* Take addresses of all TLS variables */
    use_tls_addresses(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
#ifdef __MINGW32__
        &tls_imported,
#else
        &tls_regular,
#endif
        &tls_volatile,
        &tls_static_uninit
    );
    
    /* Test external linkage simulation */
    test_external_linkage();
    
    /* Compute result using TLS variables */
    result = compute_checksum(argc);
    
    /* Create side effect based on result */
    if (result % 2) {
        tls_extern = 'X';
    } else {
        tls_extern = 'Y';
    }
    
    /* Final computation to prevent dead code elimination */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak % 256;
    result += tls_hidden % 256;
    result += tls_protected % 256;
    
#ifdef __MINGW32__
    result += tls_imported % 256;
#else
    result += tls_regular % 256;
#endif
    
    result += (int)tls_volatile;
    result += tls_static_uninit % 256;
    
    /* Return value depends on all TLS variables */
    return result % 256;
}

/* Additional file-like separation in same compilation unit */
/* This tests DECL_EXTERNAL behavior */
__thread int external_like_tls;
NOINLINE static void init_external_like(void)
{
    external_like_tls = 8888;
}
