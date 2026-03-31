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

/* Pattern E: DLL import simulation (for MinGW targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different types and attributes */
__thread volatile double tls_volatile = 3.14159;
static __thread unsigned tls_static_uninit;

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink;
    sink = (int)((uintptr_t)addr1 ^ (uintptr_t)addr2 ^ 
                 (uintptr_t)addr3 ^ (uintptr_t)addr4 ^
                 (uintptr_t)addr5 ^ (uintptr_t)addr6);
}

/* Another helper to ensure TLS variables are used in control flow */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    tls_static_init = argc * 2;
    result += tls_static_init;
    
    tls_extern = 'A' + (argc % 26);
    result += tls_extern;
    
    if (tls_weak != 0) {
        tls_weak = argc * 3;
        result += tls_weak;
    }
    
    tls_hidden = argc * 100L;
    result += (int)tls_hidden;
    
    tls_protected = argc * 200L;
    result += (int)tls_protected;
    
    tls_imported = argc * 50;
    result += tls_imported;
    
    tls_volatile = argc * 1.5;
    result += (int)tls_volatile;
    
    tls_static_uninit = argc * 7;
    result += tls_static_uninit;
    
    return result;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Force initialization and usage of all TLS variables */
    result = compute_with_tls(argc);
    
    /* Take addresses of all TLS variables to force emulation structures */
    use_tls_addresses(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported
    );
    
    /* Additional address usage */
    use_tls_addresses(
        &tls_volatile,
        &tls_static_uninit,
        &tls_static_init,  /* Reuse to ensure all are live */
        &tls_extern,
        &tls_hidden,
        &tls_protected
    );
    
    /* Complex control flow based on TLS values */
    for (int i = 0; i < argc; i++) {
        tls_static_uninit += i;
        if (tls_static_uninit % 3 == 0) {
            tls_weak += tls_static_init;
        } else {
            tls_hidden += tls_protected;
        }
        
        /* Volatile access prevents optimization */
        tls_volatile += 0.1;
    }
    
    /* Final computation using all TLS variables */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)tls_hidden;
    result += (int)tls_protected;
    result += tls_imported;
    result += (int)tls_volatile;
    result += tls_static_uninit;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d (argc=%d)\n", result, argc);
    
    return result % 256;
}
