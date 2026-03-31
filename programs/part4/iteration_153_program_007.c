/* test_emutls.c - Comprehensive TLS emulation test */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls_executable */
/* Or for 32-bit: gcc -m32 -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls_executable */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* Definition after extern declaration */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 200;
__thread long tls_protected __attribute__((visibility("protected"))) = 300;

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 400;
#else
/* Fallback for non-Windows targets - use weak instead */
__thread int tls_imported __attribute__((weak)) = 400;
#endif

/* Additional patterns for comprehensive coverage */
/* Common TLS variable (tentative definition) */
__thread double tls_common;

/* Public TLS variable with explicit visibility */
__thread float tls_public_default __attribute__((visibility("default")));

/* Used attribute test */
__thread volatile int tls_used __attribute__((used)) = 500;

/* Noinline helper function to ensure addresses are taken */
__attribute__((noinline)) 
static void use_tls_pointers(void *ptr1, void *ptr2, void *ptr3, 
                             void *ptr4, void *ptr5, void *ptr6,
                             void *ptr7, void *ptr8, void *ptr9) {
    /* Dummy writes to prevent optimization */
    volatile static int dummy = 0;
    if (ptr1) dummy++;
    if (ptr2) dummy++;
    if (ptr3) dummy++;
    if (ptr4) dummy++;
    if (ptr5) dummy++;
    if (ptr6) dummy++;
    if (ptr7) dummy++;
    if (ptr8) dummy++;
    if (ptr9) dummy++;
}

/* Another noinline function to use TLS values */
__attribute__((noinline))
static long compute_with_tls(int argc) {
    long result = 0;
    
    /* Use all TLS variables in computations */
    result += tls_static_init * argc;
    result += tls_extern;
    result += tls_weak / (argc > 0 ? argc : 1);
    result += tls_hidden % 137;
    result += tls_protected & 0xFF;
    result += tls_imported | 0x55;
    result += (long)(tls_common * 100.0);
    result += (long)(tls_public_default * 10.0f);
    result += tls_used ^ 0xAA;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile long total = 0;
    
    /* Ensure all TLS variables are used and modified */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    tls_imported = argc * 50;
    tls_common = argc * 3.14159;
    tls_public_default = argc * 2.71828f;
    tls_used = argc * 60;
    
    /* Take addresses of all TLS variables */
    void *addrs[] = {
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_common,
        &tls_public_default,
        &tls_used
    };
    
    /* Pass addresses to noinline function */
    use_tls_pointers(addrs[0], addrs[1], addrs[2], addrs[3], 
                     addrs[4], addrs[5], addrs[6], addrs[7], addrs[8]);
    
    /* Compute with TLS values in a loop to create control flow */
    for (int i = 0; i < argc + 1; i++) {
        total += compute_with_tls(i);
        
        /* Modify TLS variables in loop */
        if (i % 2 == 0) {
            tls_static_init += i;
            tls_hidden -= i;
        } else {
            tls_protected *= (i + 1);
            tls_weak /= (i > 0 ? i : 1);
        }
        
        /* Conditional TLS access */
        tls_extern = (i % 3 == 0) ? 'X' : 'Y';
        tls_imported = (total > 1000) ? 1 : 0;
    }
    
    /* Final computation using all TLS variables */
    total += tls_static_init;
    total += tls_extern;
    total += tls_weak;
    total += tls_hidden;
    total += tls_protected;
    total += tls_imported;
    total += (long)tls_common;
    total += (long)tls_public_default;
    total += tls_used;
    
    /* Print result to prevent optimization */
    printf("Result: %ld\n", total);
    
    return (int)(total % 256);
}

/* Additional file to test external linkage (compile separately if needed) */
#ifdef MULTI_FILE_TEST
/* test_emutls2.c */
__thread int external_tls_var = 999;

int get_external_tls(void) {
    return external_tls_var;
}
#endif
