/* Main file with various TLS variable declarations */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 1;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* DLL import simulation */
#ifdef _WIN32
__thread int tls_dllimport __attribute__((dllimport));
#else
/* Simulate with visibility and used */
__thread int tls_dllimport __attribute__((visibility("default"), used));
#endif

/* External declarations (defined in another file) */
extern __thread int tls_extern;
extern __thread int tls_extern_weak __attribute__((weak));
extern __thread int tls_extern_hidden __attribute__((visibility("hidden")));

/* Static TLS inside function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 555;
    tls_static_func++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* Public function using TLS */
void use_tls_variables(void) {
    /* Access all TLS variables to ensure they're used */
    tls_public_default++;
    tls_public_hidden += 2;
    
    if (tls_weak > 0) {
        tls_weak *= 2;
    }
    
    tls_common = tls_public_default + tls_public_hidden;
    
    /* External variables */
    tls_extern = tls_common;
    if (&tls_extern_weak != NULL) {
        tls_extern_weak = tls_extern;
    }
    tls_extern_hidden = tls_extern_weak + 1;
    
    /* Prevent optimization */
    asm volatile("" : : 
        "r"(&tls_public_default),
        "r"(&tls_public_hidden),
        "r"(&tls_weak),
        "r"(&tls_common),
        "r"(&tls_dllimport)
    );
}

/* Another function with different TLS usage pattern */
uint64_t calculate_tls_checksum(void) {
    uint64_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_public_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_extern;
    sum += tls_extern_weak;
    sum += tls_extern_hidden;
    
    /* Call function with static TLS */
    func_with_static_tls();
    
    return sum;
}

int main(void) {
    /* Initialize TLS variables */
    tls_public_default = 1;
    tls_public_hidden = 2;
    tls_weak = 3;
    tls_common = 4;
    
    /* Use TLS variables multiple times */
    for (int i = 0; i < 3; i++) {
        use_tls_variables();
        uint64_t checksum = calculate_tls_checksum();
        printf("Iteration %d, checksum: %llu\n", i, (unsigned long long)checksum);
    }
    
    /* Force address taking of all TLS variables for coverage */
    void* addresses[] = {
        &tls_public_default,
        &tls_public_hidden,
        &tls_weak,
        &tls_common,
        &tls_dllimport,
        &tls_extern,
        &tls_extern_weak,
        &tls_extern_hidden
    };
    
    asm volatile("" : : "r"(addresses));
    
    return 0;
}
