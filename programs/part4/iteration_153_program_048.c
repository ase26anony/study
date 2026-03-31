/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>

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

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 99;
static __thread float tls_static_uninit;

/* Helper function to force address usage */
__attribute__((noinline)) 
static void use_tls_address(void *addr) {
    /* Dummy operation to prevent optimization */
    volatile static int sink;
    sink = *(int*)addr;  /* Read to force materialization */
    (void)sink;
}

/* Another helper to ensure variables are used */
__attribute__((noinline))
static int compute_checksum(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computations */
    tls_static_init = argc * 2;
    sum += tls_static_init;
    
    tls_extern = 'A' + (argc % 26);
    sum += tls_extern;
    
    if (tls_weak) {
        tls_weak = argc * 3;
        sum += tls_weak;
    }
    
    tls_hidden = argc * 4L;
    sum += tls_hidden;
    
    tls_protected = argc * 5L;
    sum += tls_protected;
    
    tls_imported = argc * 6;
    sum += tls_imported;
    
    tls_volatile = argc * 7;
    sum += tls_volatile;
    
    tls_static_uninit = argc * 8.0f;
    sum += (int)tls_static_uninit;
    
    return sum;
}

/* Function to take addresses of all TLS variables */
__attribute__((noinline))
static void take_all_addresses(void) {
    use_tls_address(&tls_static_init);
    use_tls_address(&tls_extern);
    use_tls_address(&tls_weak);
    use_tls_address(&tls_hidden);
    use_tls_address(&tls_protected);
    use_tls_address(&tls_imported);
    use_tls_address((void*)&tls_volatile);
    use_tls_address(&tls_static_uninit);
}

/* Multi-file simulation using weak symbols */
__thread int tls_tentative __attribute__((weak));  /* Tentative definition */

int main(int argc, char **argv) {
    int result;
    
    /* Ensure TLS variables are referenced */
    if (argc > 1) {
        /* Use command-line argument to make behavior variable */
        result = compute_checksum(argc);
    } else {
        result = compute_checksum(1);
    }
    
    /* Force address taking (important for emulation) */
    take_all_addresses();
    
    /* Use tentative TLS variable */
    tls_tentative = result % 100;
    result += tls_tentative;
    
    /* Create control flow dependencies on TLS values */
    if (tls_static_init > 50) {
        tls_extern = 'X';
    }
    
    volatile int output = result;  /* Prevent optimization */
    
    /* Print to prevent dead code elimination */
    printf("TLS test result: %d\n", output);
    
    return output % 256;
}
