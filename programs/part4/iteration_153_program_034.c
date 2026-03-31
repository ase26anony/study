/* test_emutls.c
 * 
 * This test program is designed to trigger TLS emulation in GCC's tree-emutls.cc,
 * specifically covering the attribute copying lines (295-304).
 * It declares multiple TLS variables with diverse attributes to ensure
 * all relevant DECL_* flags are copied during emulation.
 */

/* Prevent inlining to ensure addresses are taken and used */
#define NOINLINE __attribute__((noinline))

/* Helper function that forces TLS variable addresses to be materialized */
NOINLINE static void use_tls_pointers(
    volatile int *a, volatile char *b, volatile int *c,
    volatile long *d, volatile long *e, volatile int *f)
{
    /* Dummy writes to prevent optimization */
    if (a) *a += 1;
    if (b) *b += 1;
    if (c) *c += 1;
    if (d) *d += 1;
    if (e) *e += 1;
    if (f) *f += 1;
}

/* Pattern A: Static TLS with initialization */
static __thread int tls_static_init = 42;

/* Pattern B: Extern TLS with common linkage */
extern __thread char tls_extern;
__thread char tls_extern = 'X';  /* Definition */

/* Pattern C: Weak TLS */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 500;
#else
/* On non-Windows, use a regular TLS variable with external linkage */
extern __thread int tls_imported;
__thread int tls_imported = 500;
#endif

/* Additional TLS variable with no special attributes for completeness */
__thread double tls_plain = 3.14159;

int main(int argc, char **argv) {
    int result = 0;
    
    /* Use all TLS variables to prevent optimization */
    
    /* Pattern A */
    tls_static_init = argc * 2;
    result += tls_static_init;
    
    /* Pattern B */
    tls_extern = 'A' + (argc % 26);
    result += tls_extern;
    
    /* Pattern C */
    if (tls_weak != 0) {
        tls_weak = argc * 3;
        result += tls_weak;
    }
    
    /* Pattern D */
    tls_hidden = argc * 100L;
    tls_protected = argc * 200L;
    result += (int)(tls_hidden + tls_protected);
    
    /* Pattern E */
    tls_imported = argc * 5;
    result += tls_imported;
    
    /* Plain TLS */
    tls_plain = argc * 1.5;
    result += (int)tls_plain;
    
    /* Take addresses of all TLS variables and pass to helper function */
    /* This forces the compiler to materialize their addresses, which is
       crucial for TLS emulation code generation */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported
    );
    
    /* Use the result in a way that depends on argc to prevent optimization */
    if (result > 1000) {
        return result % 256;
    } else {
        return result;
    }
}

/* Additional function that references TLS variables to ensure they're used */
NOINLINE static void secondary_tls_use(void) {
    volatile int dummy = tls_static_init + tls_extern + tls_weak;
    dummy += tls_hidden + tls_protected + tls_imported;
    dummy += (int)tls_plain;
    
    /* Prevent unused variable warning */
    (void)dummy;
}
