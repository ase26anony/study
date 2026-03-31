/* tls_main.c - Main file with various TLS variable declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC tls_model emulated
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* External declarations (defined in tls_aux.c) */
extern __thread int tls_external_var;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_dllimport_var __attribute__((dllimport));

/* Static TLS within function context */
static void test_static_context(void) {
    /* TLS with function context - different DECL_CONTEXT */
    static __thread int tls_in_function = 300;
    tls_in_function++;
    asm volatile("" : : "r"(&tls_in_function)); /* Prevent optimization */
}

/* Public function that uses TLS */
void __attribute__((visibility("default"))) use_public_tls(void) {
    tls_public_default++;
    tls_public_hidden += 2;
    printf("Public TLS values: %d, %d\n", tls_public_default, tls_public_hidden);
}

/* Weak function using weak TLS */
void __attribute__((weak)) use_weak_tls(void) {
    if (&tls_weak_var) { /* Ensure weak symbol is referenced */
        tls_weak_var *= 2;
    }
}

/* Function using external TLS */
void use_external_tls(void) {
    tls_external_var += 10;
#ifdef _WIN32
    /* DLL import attribute only meaningful on Windows */
    tls_dllimport_var += 5;
#endif
}

/* Function using common TLS */
void use_common_tls(void) {
    tls_common++;
}

/* Compute checksum of all TLS variables */
uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_public_hidden;
    sum += tls_weak_var;
    sum += tls_common;
    sum += tls_external_var;
    
    test_static_context(); /* Ensure function-context TLS is used */
    
    return sum;
}

int main(void) {
    /* Initialize common TLS */
    tls_common = 500;
    
    /* Use all TLS variables to ensure they're instantiated */
    use_public_tls();
    use_weak_tls();
    use_external_tls();
    use_common_tls();
    
    /* Force address-taking to prevent optimization */
    asm volatile("" : : 
        "r"(&tls_public_default),
        "r"(&tls_public_hidden),
        "r"(&tls_weak_var),
        "r"(&tls_common),
        "r"(&tls_external_var)
    );
    
    uint32_t checksum = compute_tls_checksum();
    printf("TLS checksum: %u\n", checksum);
    
    return 0;
}
