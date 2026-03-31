/* Main file with various TLS variable declarations */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC target("tls,emulated-tls")
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 1;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_dllimport __attribute__((dllimport));

/* Static TLS with different context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 500;
    tls_static_func++;
    /* Force address taken */
    asm volatile("" : : "r"(&tls_static_func));
}

/* TLS pointer with complex usage */
__thread void* tls_pointer __attribute__((used));

/* Force preservation of all TLS variables */
static void force_usage(void) {
    /* Take addresses to prevent optimization */
    volatile int* ptrs[] = {
        &tls_public_default,
        &tls_public_hidden,
        &tls_weak_var,
        &tls_common,
        &tls_external,
        &tls_external_weak,
        &tls_dllimport
    };
    
    /* Use all TLS variables */
    tls_public_default++;
    tls_public_hidden += 2;
    tls_weak_var *= 3;
    tls_common = tls_public_default + tls_public_hidden;
    
    /* Use the pointer */
    tls_pointer = (void*)(uintptr_t)tls_public_default;
    
    /* Call function with static TLS */
    func_with_static_tls();
}

/* Another function with different context */
__attribute__((noinline)) void modify_tls(void) {
    tls_public_default += 1000;
    tls_public_hidden -= 50;
    tls_weak_var = 999;
    tls_common = tls_public_default + tls_weak_var;
    
    /* Force external TLS usage */
    if (tls_external > 0) {
        tls_common += tls_external;
    }
}

/* Checksum calculation */
static uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_public_hidden;
    sum += tls_weak_var;
    sum += tls_common;
    
    /* External variables might be 0 if not linked properly */
    sum += (uintptr_t)tls_pointer;
    
    return sum;
}

int main(void) {
    /* Initialize common TLS */
    tls_common = 10;
    
    /* Force usage in main */
    force_usage();
    
    /* Modify through another function */
    modify_tls();
    
    /* Calculate and print checksum */
    uint32_t sum = tls_checksum();
    printf("TLS checksum: %u\n", (unsigned)sum);
    
    /* Verify thread-local behavior */
    printf("tls_public_default: %d\n", tls_public_default);
    printf("tls_public_hidden: %d\n", tls_public_hidden);
    printf("tls_weak_var: %d\n", tls_weak_var);
    printf("tls_common: %d\n", tls_common);
    
    return 0;
}
