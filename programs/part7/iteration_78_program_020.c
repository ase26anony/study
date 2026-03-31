/* tls_main.c - Main file with various TLS declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")  /* Prevent optimization removing declarations */
#endif

/* Public TLS with explicit visibility */
__attribute__((visibility("default")))
__thread int tls_public_default = 42;

/* Hidden visibility TLS */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;

/* Weak TLS definition */
__attribute__((weak))
__thread int tls_weak = 200;

/* Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* Used attribute to ensure preservation */
__attribute__((used))
__thread int tls_used = 300;

/* External declarations (defined in tls_aux.c) */
extern __thread int tls_extern;
extern __thread int tls_extern_weak __attribute__((weak));
extern __thread int tls_dllimport __attribute__((dllimport));

/* Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 500;
    tls_func_static++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_func_static));
}

/* Public TLS with common linkage in another context */
__thread int tls_public_common;

/* Function to use all TLS variables */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_used;
    sum += tls_extern;
    sum += tls_extern_weak;
    sum += tls_dllimport;
    
    /* Access function static TLS */
    func_with_static_tls();
    
    sum += tls_public_common;
    
    return sum;
}

/* Another function with different access pattern */
void modify_tls_values(void) {
    tls_public_default *= 2;
    tls_hidden += 1;
    tls_weak -= 1;
    tls_common = tls_public_default + tls_hidden;
    tls_used ^= 0x55;
    
    if (tls_extern > 0) {
        tls_public_common = tls_extern;
    }
}

int main(void) {
    uint32_t sum1, sum2;
    
    /* Initialize common TLS */
    tls_common = 50;
    tls_public_common = 75;
    
    /* First checksum */
    sum1 = tls_checksum();
    printf("Initial TLS checksum: %u\n", sum1);
    
    /* Modify values */
    modify_tls_values();
    
    /* Second checksum */
    sum2 = tls_checksum();
    printf("Modified TLS checksum: %u\n", sum2);
    
    /* Force address taking for all TLS variables to prevent elimination */
    asm volatile("" : : 
        "r"(&tls_public_default),
        "r"(&tls_hidden),
        "r"(&tls_weak),
        "r"(&tls_common),
        "r"(&tls_used),
        "r"(&tls_extern),
        "r"(&tls_extern_weak),
        "r"(&tls_dllimport),
        "r"(&tls_public_common)
    );
    
    return (int)(sum2 - sum1);
}
