/* test_emutls.c - Test program for TLS emulation attribute copying */

/* Prevent inlining to ensure addresses are taken */
#define NOINLINE __attribute__((noinline))

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different types and storage */
__thread double tls_double = 3.14159;
static __thread volatile int tls_volatile = 99;

/* Helper function to force address usage */
NOINLINE static void use_tls_addresses(
    int *p1, char *p2, int *p3, long *p4, long *p5, 
    int *p6, double *p7, volatile int *p8)
{
    /* Dummy writes to prevent optimization */
    if (p1) *p1 += 1;
    if (p2) *p2 += 1;
    if (p3) *p3 += 1;
    if (p4) *p4 += 1;
    if (p5) *p5 += 1;
    if (p6) *p6 += 1;
    if (p7) *p7 += 0.1;
    if (p8) *(int*)p8 += 1;  /* Cast away volatile for write */
}

/* Another helper to create control flow dependencies */
NOINLINE static int compute_with_tls(int argc)
{
    int result = 0;
    
    /* Use all TLS variables in computation */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 100;
    result += tls_protected % 100;
    result += tls_imported % 100;
    result += (int)tls_double;
    result += tls_volatile;
    
    /* Create argc-dependent behavior */
    if (argc > 1) {
        tls_static_init = argc;
        tls_extern = 'A' + (argc % 26);
        tls_weak = argc * 10;
        tls_hidden = argc * 100L;
        tls_protected = argc * 200L;
        tls_imported = argc * 50;
        tls_double = argc * 0.5;
        tls_volatile = argc;
    }
    
    return result;
}

/* Function to test external linkage */
void external_use(void)
{
    /* Reference the extern variable */
    extern __thread int external_tls_ref;
    external_tls_ref = 999;
}

/* Tentative definition to test DECL_COMMON */
__thread int tls_tentative;

int main(int argc, char **argv)
{
    int result = 0;
    
    /* Initial assignments */
    tls_static_init = argc;
    tls_extern = 'a' + (argc % 26);
    tls_weak = argc * 2;
    tls_hidden = argc * 3L;
    tls_protected = argc * 4L;
    tls_imported = argc * 5;
    tls_double = argc * 0.25;
    tls_volatile = argc * 6;
    tls_tentative = argc * 7;
    
    /* Use in computation with control flow */
    for (int i = 0; i < argc; i++) {
        result += compute_with_tls(i);
        
        /* Modify TLS variables in loop */
        if (i % 2 == 0) {
            tls_static_init += i;
            tls_volatile += i;
        }
    }
    
    /* Take addresses of all TLS variables */
    use_tls_addresses(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_double,
        &tls_volatile
    );
    
    /* Also take address of tentative definition */
    int *tentative_ptr = &tls_tentative;
    *tentative_ptr += result;
    
    /* Use external reference */
    external_use();
    
    /* Final computation using all TLS variables */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 256;
    result += tls_protected % 256;
    result += tls_imported % 256;
    result += (int)tls_double;
    result += tls_volatile;
    result += tls_tentative;
    
    /* Prevent dead code elimination */
    if (result == 0) {
        /* This should never happen, but prevents optimization */
        return 1;
    }
    
    return result % 256;
}
