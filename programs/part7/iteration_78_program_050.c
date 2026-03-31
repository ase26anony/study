/* Main file with various TLS variable declarations */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC target("tls,emulated")
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 1;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* External declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_dllimport __attribute__((dllimport));

/* Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 500;
    tls_static_func++;
    /* Force address taken to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* DLL-like attribute simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Another external with DLL import attribute */
extern DLL_IMPORT __thread int tls_imported;

/* Function that uses all TLS variables */
uint64_t compute_tls_checksum(void) {
    uint64_t sum = 0;
    
    /* Access all TLS variables */
    sum += tls_public_default;
    sum += tls_public_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_external;
    
    /* Conditional access to weak external */
    if (&tls_external_weak != NULL) {
        sum += tls_external_weak;
    }
    
    /* Access function-static TLS */
    func_with_static_tls();
    
    /* Prevent elimination of unused variables */
    asm volatile("" : : "r"(&tls_public_default), 
                       "r"(&tls_public_hidden),
                       "r"(&tls_weak),
                       "r"(&tls_common),
                       "r"(&tls_external));
    
    return sum;
}

/* Thread-specific counter in TLS */
__thread unsigned long tls_thread_counter = 0;

void increment_counters(void) {
    tls_thread_counter++;
    tls_public_default++;
    tls_common += 2;
    
    /* Use in conditional logic */
    if (tls_thread_counter % 2 == 0) {
        tls_public_hidden--;
    } else {
        tls_public_hidden++;
    }
}

int main(void) {
    uint64_t checksum = 0;
    
    /* Initialize common TLS */
    tls_common = 1000;
    
    /* Access external TLS variables */
    checksum += tls_external;
    
    /* Use weak TLS */
    tls_weak = 50;
    
    /* Multiple accesses to ensure instantiation */
    for (int i = 0; i < 10; i++) {
        increment_counters();
        checksum += compute_tls_checksum();
    }
    
    /* Print results to prevent dead code elimination */
    printf("TLS checksum: %llu\n", (unsigned long long)checksum);
    printf("Thread counter: %lu\n", tls_thread_counter);
    printf("Public default: %d\n", tls_public_default);
    printf("Public hidden: %d\n", tls_public_hidden);
    printf("Weak TLS: %d\n", tls_weak);
    printf("Common TLS: %d\n", tls_common);
    
    return 0;
}
