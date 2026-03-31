/* tls_main.c - Main file with various TLS variable declarations */

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

/* Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* DLL import simulation (even on non-Windows, attribute should be copied) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* External declarations (defined in tls_aux.c) */
extern __thread int tls_external;
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 999;
    tls_static_func++;
    /* Force address taken to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* Public TLS used in calculations */
__thread uint64_t tls_counter __attribute__((used)) = 0;

/* Weak external reference */
extern __thread int tls_weak_extern __attribute__((weak));

/* Function to use all TLS variables */
void use_tls_variables(void) {
    /* Modify public TLS */
    tls_public_default += 1;
    
    /* Modify hidden TLS */
    tls_hidden *= 2;
    
    /* Modify weak TLS if defined */
    if (&tls_weak != NULL) {
        tls_weak -= 5;
    }
    
    /* Initialize common TLS */
    tls_common = 1234;
    
    /* Use DLL import TLS */
    tls_dllimport = 5678;
    
    /* Use external TLS */
    tls_external++;
    tls_external_hidden += 2;
    
    /* Increment counter */
    tls_counter++;
    
    /* Try weak external */
    if (&tls_weak_extern != NULL) {
        tls_weak_extern = 9999;
    }
    
    /* Call function with static TLS */
    func_with_static_tls();
}

/* Checksum function */
uint64_t calculate_tls_checksum(void) {
    uint64_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_dllimport;
    sum += tls_external;
    sum += tls_external_hidden;
    sum += tls_counter;
    
    return sum;
}

int main(void) {
    printf("Starting emulated TLS test...\n");
    
    /* Initial use */
    use_tls_variables();
    
    /* Second use to modify values */
    tls_public_default = 1000;
    tls_hidden = 2000;
    tls_common = 3000;
    
    use_tls_variables();
    
    /* Calculate and print checksum */
    uint64_t checksum = calculate_tls_checksum();
    printf("TLS checksum: %llu\n", (unsigned long long)checksum);
    
    /* Force all TLS addresses to be taken */
    void* addresses[] = {
        &tls_public_default,
        &tls_hidden,
        &tls_weak,
        &tls_common,
        &tls_dllimport,
        &tls_external,
        &tls_external_hidden,
        &tls_counter
    };
    
    /* Use addresses to prevent optimization */
    asm volatile("" : : "r"(addresses));
    
    printf("Test completed.\n");
    return 0;
}
