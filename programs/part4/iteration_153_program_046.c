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
/* On non-Windows, simulate with weak external */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 500;

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 99;
static __thread float tls_static_uninit;

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_address(void *addr, int val) {
    /* Dummy operation to force address materialization */
    volatile int *p = (volatile int *)addr;
    (void)p; /* Prevent unused variable warning */
    /* The compiler can't optimize away this if we use volatile */
}

/* Another helper that returns a value based on TLS variables */
__attribute__((noinline))
static int compute_tls_sum(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computations */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 100;
    sum += tls_protected % 100;
    sum += tls_imported;
    sum += tls_volatile;
    sum += (int)tls_static_uninit;
    
    /* Make result dependent on argc to prevent constant folding */
    return sum * (argc > 0 ? argc : 1);
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* 1. Assign values to TLS variables based on argc */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30;
    tls_protected = argc * 40;
    tls_imported = argc * 50;
    tls_volatile = argc * 60;
    tls_static_uninit = argc * 0.5f;
    
    /* 2. Take addresses of all TLS variables to force emulation */
    use_tls_address(&tls_static_init, argc);
    use_tls_address(&tls_extern, argc);
    use_tls_address(&tls_weak, argc);
    use_tls_address(&tls_hidden, argc);
    use_tls_address(&tls_protected, argc);
    use_tls_address(&tls_imported, argc);
    use_tls_address(&tls_volatile, argc);
    use_tls_address(&tls_static_uninit, argc);
    
    /* 3. Use TLS variables in control flow */
    if (tls_static_init > 100) {
        tls_hidden += tls_protected;
    }
    
    for (int i = 0; i < (tls_weak % 5); i++) {
        tls_volatile += i;
    }
    
    /* 4. Compute final result using helper */
    result = compute_tls_sum(argc);
    
    /* 5. Print result to prevent dead code elimination */
    printf("TLS test result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

/* Force generation of TLS initialization code */
__attribute__((constructor))
static void init_tls_vars(void) {
    /* Access TLS variables in constructor to ensure 
       they're included in startup code */
    volatile int dummy = tls_static_init + tls_extern;
    (void)dummy;
}
