/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern/public TLS variable with tentative definition */
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
__thread int tls_imported = 5000;
#else
/* Simulate similar behavior with weak attribute */
extern __thread int tls_imported __attribute__((weak));
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different types and storage */
__thread double tls_dynamic;
__thread volatile int tls_volatile;
static __thread int tls_static_uninit;

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink = 0;
    sink += (int)((uintptr_t)addr1 ^ (uintptr_t)addr2);
    sink += (int)((uintptr_t)addr3 ^ (uintptr_t)addr4);
    sink += (int)((uintptr_t)addr5 ^ (uintptr_t)addr6);
    (void)sink; /* Suppress unused warning */
}

/* Another helper to create control flow dependencies */
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
    
    tls_dynamic = argc * 3.14;
    result += (int)tls_dynamic;
    
    tls_volatile = argc * 7;
    result += tls_volatile;
    
    tls_static_uninit = argc * 11;
    result += tls_static_uninit;
    
    return result;
}

/* Function to ensure TLS variables are referenced in different scopes */
static void modify_tls_in_loop(int iterations) {
    for (int i = 0; i < iterations; i++) {
        tls_static_init += i;
        tls_dynamic += i * 0.5;
        tls_volatile ^= i;
    }
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Ensure all TLS variables are used */
    result = compute_with_tls(argc);
    
    /* Modify TLS variables in a loop */
    modify_tls_in_loop(argc > 0 ? argc : 10);
    
    /* Take addresses of all TLS variables to force emulation */
    use_tls_addresses(&tls_static_init, &tls_extern, &tls_weak,
                     &tls_hidden, &tls_protected, &tls_imported);
    
    use_tls_addresses(&tls_dynamic, &tls_volatile, &tls_static_uninit,
                     &tls_static_init, &tls_extern, &tls_weak);
    
    /* Create a value that depends on all TLS variables */
    int final_result = result 
                     + tls_static_init 
                     + tls_extern 
                     + tls_weak
                     + (int)tls_hidden
                     + (int)tls_protected
                     + tls_imported
                     + (int)tls_dynamic
                     + tls_volatile
                     + tls_static_uninit;
    
    /* Print to prevent optimization */
    printf("TLS test result: %d\n", final_result % 1000);
    
    return final_result % 256;
}
