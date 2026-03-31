/* Main file with various TLS declarations and usage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC target("tls")  /* Ensure TLS support is considered */
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 1;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* External declarations (defined in tls_aux.c) */
extern __thread int tls_external_var;
extern __thread char tls_external_array[16];
extern __thread void* tls_external_ptr __attribute__((dllimport));

/* Static TLS within function context */
static void use_local_tls(void) {
    static __thread int local_func_tls = 0;
    local_func_tls++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&local_func_tls));
}

/* Function using all TLS variables */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    /* Access all TLS variables */
    sum += tls_public_default;
    sum += tls_public_hidden;
    sum += tls_weak_var;
    sum += tls_common;
    sum += tls_external_var;
    
    /* Access external array */
    for (int i = 0; i < 16; i++) {
        sum += tls_external_array[i];
    }
    
    /* Force address taking for external ptr */
    if (&tls_external_ptr) {
        sum += 1;
    }
    
    use_local_tls();
    
    return sum;
}

/* Function that modifies TLS variables */
void modify_tls_vars(void) {
    tls_public_default *= 2;
    tls_public_hidden += 3;
    tls_weak_var -= 1;
    tls_common = tls_checksum() % 256;
}

/* Another function with different TLS usage pattern */
void* get_tls_addresses(void) {
    static void* addrs[8];
    
    addrs[0] = &tls_public_default;
    addrs[1] = &tls_public_hidden;
    addrs[2] = &tls_weak_var;
    addrs[3] = &tls_common;
    addrs[4] = &tls_external_var;
    addrs[5] = tls_external_array;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(addrs));
    
    return addrs[0];
}

int main(void) {
    uint32_t sum1, sum2;
    
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initial access */
    sum1 = tls_checksum();
    printf("Initial checksum: %u\n", sum1);
    
    /* Modify and check again */
    modify_tls_vars();
    sum2 = tls_checksum();
    printf("Modified checksum: %u\n", sum2);
    
    /* Force address calculation */
    get_tls_addresses();
    
    /* Use common linkage variable */
    tls_common = sum1 ^ sum2;
    
    printf("TLS test completed.\n");
    printf("tls_public_default: %d\n", tls_public_default);
    printf("tls_public_hidden: %d\n", tls_public_hidden);
    printf("tls_weak_var: %d\n", tls_weak_var);
    printf("tls_common: %d\n", tls_common);
    
    return 0;
}
