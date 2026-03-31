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
#else
/* Simulate similar attribute on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 500;

/* Additional patterns for comprehensive coverage */
/* Thread-local with common linkage (tentative definition) */
__thread int tls_common;  /* Tests DECL_COMMON */

/* Thread-local with used attribute */
__thread int tls_used __attribute__((used)) = 300;

/* Thread-local with preserve attribute simulation */
__thread volatile int tls_preserve = 400;  /* volatile may affect DECL_PRESERVE_P */

/* Noinline helper to force address taking and prevent optimization */
__attribute__((noinline))
static void use_tls_pointers(
    int *a, char *b, int *c, long *d, long *e, 
    int *f, int *g, int *h, int *i)
{
    /* Dummy operations to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
    if (g) *g += 1;
    if (h) *h += 1;
    if (i) *i += 1;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Another noinline function that uses TLS values */
__attribute__((noinline, optimize("O0")))
static int compute_with_tls(int argc)
{
    int result = 0;
    
    /* Use all TLS variables in computation */
    result += tls_static_init * argc;
    result += tls_extern;
    result += tls_weak / (argc + 1);  /* Prevent division by zero */
    result += tls_hidden % 256;
    result += tls_protected % 256;
    result += tls_imported;
    result += tls_common;
    result += tls_used;
    result += tls_preserve;
    
    return result;
}

int main(int argc, char **argv)
{
    int volatile result = 0;
    
    /* Force initialization and usage of all TLS variables */
    if (argc > 1) {
        /* Pattern A usage */
        tls_static_init = argc * 10;
        
        /* Pattern B usage */
        tls_extern = 'A' + (argc % 26);
        
        /* Pattern C usage */
        tls_weak = argc * 20;
        
        /* Pattern D usage */
        tls_hidden = argc * 30L;
        tls_protected = argc * 40L;
        
        /* Pattern E usage */
        tls_imported = argc * 50;
        
        /* Other patterns */
        tls_common = argc * 60;
        tls_used = argc * 70;
        tls_preserve = argc * 80;
    }
    
    /* Take addresses of all TLS variables - crucial for emulation */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_common,
        &tls_used,
        &tls_preserve
    );
    
    /* Compute using TLS values */
    result = compute_with_tls(argc);
    
    /* Use result in output to prevent dead code elimination */
    printf("TLS computation result: %d\n", result);
    
    /* Additional complex usage pattern */
    for (int i = 0; i < argc; i++) {
        /* Create control flow dependencies on TLS values */
        if (tls_static_init > 100) {
            tls_extern = 'Z';
        }
        
        if (tls_weak % 2 == 0) {
            tls_hidden += i;
        } else {
            tls_protected -= i;
        }
        
        /* Chain of dependencies */
        tls_imported = tls_common + tls_used;
        tls_preserve = tls_imported * 2;
    }
    
    /* Final computation */
    result += tls_static_init + tls_extern + tls_weak + 
              (int)tls_hidden + (int)tls_protected + 
              tls_imported + tls_common + tls_used + tls_preserve;
    
    return result % 256;  /* Return non-zero, non-constant value */
}

/* Additional file-scope TLS usage to ensure DECL_CONTEXT is set */
__thread int file_scope_tls = 123;

static void __attribute__((constructor)) init_tls(void)
{
    /* Access TLS in constructor to ensure early initialization */
    file_scope_tls = 456;
}

static void __attribute__((destructor)) cleanup_tls(void)
{
    /* Access TLS in destructor */
    file_scope_tls = 0;
}
