/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern/public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 999;
static __thread float tls_static_uninit;

/* Helper function to ensure addresses are taken and used */
__attribute__((noinline)) 
static void use_tls_pointers(void *p1, void *p2, void *p3, void *p4, void *p5) {
    /* Dummy writes to prevent optimization */
    volatile static int dummy = 0;
    if (p1) dummy++;
    if (p2) dummy++;
    if (p3) dummy++;
    if (p4) dummy++;
    if (p5) dummy++;
}

/* Another helper to create control flow dependencies */
__attribute__((noinline))
static int compute_checksum(int a, int b, int c, long d, long e) {
    return (a ^ b ^ c) + (int)(d % 1000) + (int)(e % 1000);
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* 1. Assign values to TLS variables with argc dependency */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    tls_imported = argc * 50;
    tls_volatile = argc * 60;
    tls_static_uninit = argc * 70.0f;
    
    /* 2. Use TLS values in computations */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)(tls_hidden % 100);
    result += (int)(tls_protected % 100);
    result += tls_imported;
    result += tls_volatile;
    result += (int)tls_static_uninit;
    
    /* 3. Take addresses of all TLS variables */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected
    );
    
    use_tls_pointers(
        &tls_imported,
        &tls_volatile,
        &tls_static_uninit,
        (void*)&tls_hidden,  /* Cast to avoid type warnings */
        (void*)&tls_protected
    );
    
    /* 4. Create control flow based on TLS values */
    if (tls_static_init > 100) {
        tls_weak += tls_static_init;
    }
    
    for (int i = 0; i < (tls_volatile % 5); i++) {
        tls_hidden += i;
    }
    
    /* 5. Final computation using helper function */
    result = compute_checksum(
        tls_static_init,
        tls_weak,
        tls_imported,
        tls_hidden,
        tls_protected
    );
    
    /* 6. Prevent dead code elimination */
    volatile int final_output = result;
    
    /* Use the result in a way that can't be optimized away */
    printf("TLS emulation test result: %d\n", result);
    
    return result != 0 ? 0 : 1;  /* Non-zero return if all zeros (unlikely) */
}

/* Additional file-like separation using weak symbols */
/* This creates a "tentative definition" scenario */
__thread int tls_tentative __attribute__((weak));

/* Function in another "compilation unit" using the TLS */
__attribute__((noinline))
static void another_function(void) {
    /* Access tentative TLS variable */
    if (tls_tentative == 0) {
        tls_tentative = 1234;
    }
    
    /* Access all TLS variables to ensure they're referenced */
    volatile int dummy = tls_static_init + tls_extern + tls_weak;
    (void)dummy;  /* Suppress unused warning */
}

/* Call from main to ensure function is used */
void call_another_function(void) {
    another_function();
}
