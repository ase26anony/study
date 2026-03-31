/* tls_main.c - Main file with various TLS declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#pragma GCC tls_model emulated

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* DLL import simulation (for attribute testing) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_imported;
#else
    /* Simulate dllimport attribute on non-Windows */
    __thread int tls_imported __attribute__((dllimport));
#endif

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* Function prototypes from other files */
void use_tls_variables(void);
void modify_tls_variables(void);
uint32_t compute_tls_checksum(void);

/* Static function with local TLS context */
static void static_function_with_tls(void) {
    /* TLS with function context */
    static __thread int tls_in_function = 999;
    tls_in_function++;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&tls_in_function));
}

/* Public function using TLS */
void public_function(void) {
    tls_public_default++;
    tls_hidden += 2;
    tls_weak += 3;
    tls_common += 4;
    
    static_function_with_tls();
    
    /* Use external TLS */
    if (tls_external > 0) {
        tls_external++;
    }
}

/* Another public function for more coverage */
void another_public_function(void) {
    /* Different access pattern */
    tls_public_default *= 2;
    tls_hidden /= 2;
    tls_weak |= 0xFF;
    tls_common ^= 0x55;
    
    /* Force address taking without side effects */
    asm volatile("" : : "r"(&tls_imported));
}

int main(void) {
    uint32_t checksum;
    
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initialize common TLS */
    tls_common = 50;
    
    /* Use all TLS variables */
    public_function();
    another_public_function();
    static_function_with_tls();
    
    /* Call functions from other translation units */
    use_tls_variables();
    modify_tls_variables();
    
    /* Compute and print checksum */
    checksum = compute_tls_checksum();
    printf("TLS checksum: 0x%08X\n", checksum);
    
    /* Additional access patterns */
    for (int i = 0; i < 3; i++) {
        tls_public_default += i;
        tls_hidden -= i;
        tls_weak *= (i + 1);
        tls_common = tls_common ^ tls_public_default;
        
        if (i % 2 == 0) {
            static_function_with_tls();
        }
    }
    
    /* Final checksum */
    checksum = compute_tls_checksum();
    printf("Final TLS checksum: 0x%08X\n", checksum);
    
    return 0;
}
