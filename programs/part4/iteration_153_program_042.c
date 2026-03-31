/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

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
/* Simulate with weak external on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 5000;

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 999;
static __thread float tls_static_uninit;

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    /* Dummy writes to force materialization of addresses */
    volatile static int sink;
    sink = (addr1 != NULL);
    sink = (addr2 != NULL);
    sink = (addr3 != NULL);
    sink = (addr4 != NULL);
    sink = (addr5 != NULL);
    sink = (addr6 != NULL);
}

/* Another helper that uses TLS values */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    result += tls_static_init * 2;
    result += tls_extern;
    result += tls_weak / 2;
    result += tls_hidden % 100;
    result += tls_protected >> 2;
    result += tls_imported & 0xFF;
    result += tls_volatile;
    
    /* Conditional based on TLS values */
    if (tls_static_init > argc) {
        result += 1000;
    }
    
    /* Loop that depends on TLS */
    for (int i = 0; i < (tls_weak % 10); i++) {
        result += i;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Modify TLS variables based on program input */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 100;
    tls_hidden = argc * 1000L;
    tls_protected = argc * 2000L;
    tls_imported = argc * 5000;
    tls_volatile = argc * 999;
    tls_static_uninit = argc * 3.14f;
    
    /* Force address-taking of all TLS variables */
    use_tls_addresses(&tls_static_init, &tls_extern, &tls_weak,
                      &tls_hidden, &tls_protected, &tls_imported);
    use_tls_addresses(&tls_volatile, &tls_static_uninit, 
                      (void*)(ptrdiff_t)tls_static_init,
                      (void*)(ptrdiff_t)tls_extern,
                      (void*)(ptrdiff_t)tls_weak,
                      (void*)(ptrdiff_t)tls_hidden);
    
    /* Compute with TLS values */
    result = compute_with_tls(argc);
    
    /* Use TLS in switch statement */
    switch (tls_extern % 4) {
        case 0: result += tls_static_init; break;
        case 1: result += tls_weak; break;
        case 2: result += tls_hidden; break;
        case 3: result += tls_protected; break;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Return value depends on TLS state */
    return (result > 1000) ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
#ifdef USE_MULTIFILE_SIMULATION
/* Simulate external reference from another compilation unit */
extern __thread int external_tls_ref __attribute__((weak));
__thread int external_tls_ref = 7777;

/* Function that could be in another file */
__attribute__((noinline)) 
static void cross_file_tls_use(void) {
    external_tls_ref = tls_static_init + tls_weak;
}
#endif
