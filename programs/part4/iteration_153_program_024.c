/* test_emutls.c - Test program for GCC TLS emulation attribute copying */

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
/* Simulate similar behavior with weak attribute */
__thread int tls_imported __attribute__((weak)) = 500;
#endif

/* Additional patterns for comprehensive coverage */
/* File-local static TLS with no initializer */
static __thread double tls_static_uninit;

/* Public TLS with common linkage simulation */
__thread int tls_common __attribute__((common));

/* Noinline helper function to ensure addresses are taken and used */
__attribute__((noinline)) 
static void use_tls_pointers(
    int *p1, char *p2, int *p3, long *p4, long *p5, int *p6, double *p7, int *p8) 
{
    /* Dummy operations to prevent optimization */
    if (p1) *p1 ^= 0x12345678;
    if (p2) *p2 ^= 0x55;
    if (p3) *p3 += 1;
    if (p4) *p4 |= 0x1000;
    if (p5) *p5 &= ~0x800;
    if (p6) *p6 *= 2;
    if (p7) *p7 += 0.5;
    if (p8) *p8 |= 0x1;
    
    /* Volatile asm to prevent optimization */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5), "r"(p6), "r"(p7), "r"(p8) : "memory");
}

/* Another noinline function to use TLS values directly */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    volatile int result = 0;
    
    /* Use all TLS variables in computation */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 100;
    result += tls_protected % 100;
    result += tls_imported;
    result += (int)tls_static_uninit;
    result += tls_common;
    
    /* Make result dependent on argc */
    return result * (argc + 1);
}

int main(int argc, char **argv) {
    int i;
    
    /* Initialize uninitialized TLS variables */
    tls_static_uninit = 3.14159;
    tls_common = 999;
    
    /* Modify TLS variables based on argc */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 100;
    tls_hidden = 5000 + argc;
    tls_protected = 6000 + argc;
    tls_imported = 7000 + argc;
    
    /* Take addresses of all TLS variables */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_static_uninit,
        &tls_common
    );
    
    /* Use TLS variables in loops to create control flow dependencies */
    for (i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_common -= i;
        
        /* Conditional based on TLS values */
        if (tls_hidden > 5000) {
            tls_protected += tls_weak;
        }
    }
    
    /* Compute final result using TLS values */
    int final_result = compute_with_tls(argc);
    
    /* Use result to prevent dead code elimination */
    if (final_result > 1000000) {
        return 1;
    }
    
    return final_result % 256;
}

/* Additional file to test external linkage (compile separately if needed) */
#ifdef COMPILE_SECOND_FILE
/* test_emutls2.c */
extern __thread int tls_weak;
extern __thread char tls_extern;

int helper_function(void) {
    return tls_weak + tls_extern;
}
#endif
