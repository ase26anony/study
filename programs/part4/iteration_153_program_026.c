/* test_emutls.c - Test program for TLS emulation attribute copying */

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
__thread int tls_imported = 5000;

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 300;
static __thread float tls_static_uninit;

/* Helper function to ensure addresses are taken and used */
__attribute__((noinline)) 
static void use_tls_pointers(
    int *p1, char *p2, int *p3, long *p4, long *p5, int *p6, 
    int *p7, float *p8, int *p9
) {
    /* Dummy operations to prevent optimization */
    if (p1) *p1 += 1;
    if (p2) *p2 += 1;
    if (p3) *p3 += 1;
    if (p4) *p4 += 1;
    if (p5) *p5 += 1;
    if (p6) *p6 += 1;
    if (p7) *p7 += 1;
    if (p8) *p8 += 1.0f;
    if (p9) *p9 += 1;
    
    /* Create side effect to prevent dead code elimination */
    static int counter = 0;
    counter++;
}

/* Another helper to use TLS values in computation */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computation */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 100;
    sum += tls_protected % 100;
    sum += tls_imported % 100;
    sum += tls_volatile;
    sum += (int)tls_static_uninit;
    
    /* Make result dependent on argc */
    return sum * (argc + 1);
}

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
    
    /* Other TLS variables */
    tls_volatile = argc * 60;
    tls_static_uninit = argc * 70.0f;
    
    /* Additional TLS variable defined in main to test DECL_CONTEXT */
    static __thread int tls_local_main = 99;
    tls_local_main = argc * 80;
    
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
        &tls_local_main
    );
    
    /* Use TLS values in computation with control flow */
    for (int i = 0; i < argc; i++) {
        result += compute_with_tls(i);
        
        /* Modify TLS variables in loop */
        if (i % 2 == 0) {
            tls_static_init += i;
            tls_hidden -= i;
        } else {
            tls_protected += i;
            tls_volatile ^= i;
        }
    }
    
    /* Final computation using all TLS variables */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)(tls_hidden % 256);
    result += (int)(tls_protected % 256);
    result += tls_imported;
    result += tls_volatile;
    result += (int)tls_static_uninit;
    result += tls_local_main;
    
    /* Return value depends on TLS state */
    return result % 256;
}

/* Additional file-like separation using weak symbols */
/* This tests DECL_EXTERNAL and DECL_COMMON behavior */
__thread int tls_tentative;  /* Tentative definition - tests DECL_COMMON */

/* External declaration that gets defined later */
extern __thread double tls_external_def;
__thread double tls_external_def = 3.14159;

/* Function in another compilation unit simulation */
void __attribute__((weak)) use_more_tls(void) {
    /* Reference tentative definition to ensure it's kept */
    tls_tentative = 123;
    
    /* Use external definition */
    tls_external_def += 1.0;
}
