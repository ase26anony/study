/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stdint.h>

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
/* Simulate with weak linkage on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 5000;

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 300;
static __thread float tls_static_uninit;

/* Helper function to force address usage */
__attribute__((noinline)) 
static void use_tls_address(void *addr, int idx) {
    /* Dummy operation to prevent optimization */
    volatile static int sink;
    if (addr) {
        sink = idx;
    }
}

/* Another helper to ensure variables are used */
__attribute__((noinline))
static int compute_checksum(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computations */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 256;
    sum += tls_protected % 256;
    sum += tls_imported % 256;
    sum += tls_volatile;
    sum += (int)tls_static_uninit;
    
    /* Make result dependent on argc */
    return sum * (argc + 1);
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Modify TLS variables based on argc */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    tls_imported = argc * 50;
    tls_volatile = argc * 60;
    tls_static_uninit = argc * 70.0f;
    
    /* Force taking addresses of all TLS variables */
    use_tls_address(&tls_static_init, 1);
    use_tls_address(&tls_extern, 2);
    use_tls_address(&tls_weak, 3);
    use_tls_address(&tls_hidden, 4);
    use_tls_address(&tls_protected, 5);
    use_tls_address(&tls_imported, 6);
    use_tls_address(&tls_volatile, 7);
    use_tls_address(&tls_static_uninit, 8);
    
    /* Use TLS variables in computation */
    result = compute_checksum(argc);
    
    /* Conditional use to prevent dead code elimination */
    if (result > 1000) {
        printf("TLS test result: %d\n", result);
    } else {
        printf("TLS test result (low): %d\n", result);
    }
    
    return result % 256;
}

/* Additional file-like separation using weak symbols */
/* This creates multiple declarations to test DECL_EXTERNAL and DECL_COMMON */
__thread int tls_tentative;  /* Tentative definition */
extern __thread int tls_tentative;  /* External declaration */

/* Function in another "compilation unit" */
static void another_function(void) {
    /* Reference TLS variables from different context */
    volatile int local = tls_static_init + tls_weak;
    (void)local;
}
