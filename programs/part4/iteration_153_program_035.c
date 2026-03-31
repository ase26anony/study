/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

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
__thread int tls_imported = 5000;

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 300;
static __thread float tls_static_uninit;

/* Helper function to ensure addresses are taken and used */
__attribute__((noinline, used))
static void use_tls_pointers(
    int *p1, char *p2, int *p3, long *p4, long *p5, int *p6,
    volatile int *p7, float *p8)
{
    /* Dummy operations to prevent optimization */
    if (p1) *p1 += 1;
    if (p2) *p2 = (*p2 == 0) ? 'A' : *p2 + 1;
    if (p3) *p3 ^= 0x55;
    if (p4) *p4 = (*p4 >> 1) | (*p4 << (sizeof(long)*8 - 1));
    if (p5) *p5 += *p4;
    if (p6) *p6 = (*p6 * 2) / 2; /* Identity operation but not optimized away */
    if (p7) (void)*p7; /* Read volatile */
    if (p8) *p8 = *p8 + 0.5f;
    
    /* Create side effect */
    static int counter = 0;
    counter++;
}

/* Another helper that returns a value based on TLS variables */
__attribute__((noinline, used))
static int compute_from_tls(void)
{
    int sum = tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 256;
    sum += tls_protected % 256;
    sum += tls_imported % 256;
    sum += tls_volatile % 256;
    sum += (int)tls_static_uninit;
    
    /* Make result dependent on all TLS variables */
    return sum & 0xFF;
}

/* Function that uses TLS in different scopes */
__attribute__((noinline, used))
static void modify_tls_values(int argc)
{
    /* Modify Pattern A */
    tls_static_init = argc * 10;
    
    /* Modify Pattern B */
    tls_extern = 'A' + (argc % 26);
    
    /* Modify Pattern C if strong definition exists */
    if (&tls_weak)
        tls_weak = argc * 100;
    
    /* Modify Pattern D */
    tls_hidden = argc * 1000L;
    tls_protected = argc * 2000L;
    
    /* Modify Pattern E */
    tls_imported = argc * 5000;
    
    /* Modify other TLS variables */
    tls_volatile = argc * 300;
    tls_static_uninit = argc * 3.14f;
}

/* Main function with control flow that depends on TLS */
int main(int argc, char **argv)
{
    int result = 0;
    
    /* Initial modification based on argc */
    modify_tls_values(argc);
    
    /* Take addresses of all TLS variables */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_volatile,
        &tls_static_uninit
    );
    
    /* Loop that creates control flow dependency on TLS */
    for (int i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_extern = (tls_extern + 1) & 0x7F;
        
        /* Conditional based on TLS values */
        if (tls_weak > 1000) {
            tls_hidden >>= 1;
        } else {
            tls_protected <<= 1;
        }
        
        /* Volatile access prevents optimization */
        result += tls_volatile;
    }
    
    /* Compute final result using TLS */
    result += compute_from_tls();
    
    /* Use result to affect return value */
    return result % 256;
}

/* Additional file-like separation using weak symbols */
/* This tests DECL_COMMON and DECL_EXTERNAL behavior */
__thread int tls_tentative;  /* Tentative definition - tests DECL_COMMON */

/* Force multiple translation units in one file */
#ifdef FORCE_MULTIFILE
/* Simulate external reference from another TU */
extern __thread int external_tls_ref;

/* And a definition that might be common */
__thread int external_tls_ref = 9999;
#endif
