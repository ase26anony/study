/* tls_main.c - Main file with various TLS declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")  /* Prevent optimization removing TLS vars */
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int tls_external;
extern __thread char tls_external_char;
extern DLL_IMPORT __thread int tls_dllimport;

/* Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 999;
    tls_func_static++;
    /* Force address taken to prevent optimization */
    asm volatile("" : : "r"(&tls_func_static));
}

/* Public TLS used for thread ID */
__thread uintptr_t thread_specific_id __attribute__((used));

/* Weak external reference */
extern __thread int tls_weak_extern __attribute__((weak));

/* Function using all TLS variables to prevent elimination */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    /* Access all TLS variables */
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_external;
    sum += tls_external_char;
    
    /* Conditional access to ensure live range */
    if (tls_public_default > 0) {
        tls_hidden = tls_public_default * 2;
    }
    
    /* Use thread ID */
    thread_specific_id = (uintptr_t)&sum;
    sum += (thread_specific_id & 0xFFFF);
    
    func_with_static_tls();
    
    return sum;
}

/* Another function with different access pattern */
void modify_tls_values(void) {
    tls_public_default++;
    tls_hidden--;
    tls_weak *= 2;
    tls_common = tls_public_default + tls_hidden;
    
    /* Force external TLS usage */
    if (tls_external > 0) {
        tls_external_char = 'A' + (tls_external % 26);
    }
    
    /* Handle weak external if it exists */
    if (&tls_weak_extern != NULL) {
        tls_weak_extern++;
    }
}

int main(void) {
    uint32_t checksum = 0;
    
    /* Initialize common TLS */
    tls_common = 50;
    
    /* First access to ensure TLS instantiation */
    checksum = tls_checksum();
    printf("Initial TLS checksum: %u\n", checksum);
    
    /* Modify and recalculate */
    for (int i = 0; i < 3; i++) {
        modify_tls_values();
        uint32_t new_sum = tls_checksum();
        printf("Iteration %d checksum: %u\n", i, new_sum);
        checksum += new_sum;
    }
    
    /* Final verification */
    printf("Final cumulative checksum: %u\n", checksum);
    
    /* Print addresses to ensure TLS vars are unique */
    printf("TLS addresses:\n");
    printf("  tls_public_default: %p\n", (void*)&tls_public_default);
    printf("  tls_hidden: %p\n", (void*)&tls_hidden);
    printf("  tls_weak: %p\n", (void*)&tls_weak);
    printf("  tls_common: %p\n", (void*)&tls_common);
    
    return (checksum > 0) ? 0 : 1;
}
