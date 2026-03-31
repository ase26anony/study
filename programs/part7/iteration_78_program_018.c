/* tls_main.c - Main file with various TLS variable declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#pragma GCC tls_model emulated

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* DLL import simulation (for attribute testing) */
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
#else
    #define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int tls_external_var;
extern __thread char tls_external_array[16];
extern __thread uint64_t tls_external_weak __attribute__((weak));

/* Static TLS within function context */
static void use_static_tls(void) {
    static __thread int tls_static_func __attribute__((used)) = 300;
    tls_static_func++;
    /* Force address taking without side effects */
    asm volatile("" : : "r"(&tls_static_func));
}

/* Function using all TLS variables */
uint64_t compute_tls_checksum(void) {
    uint64_t sum = 0;
    
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
    
    /* Access weak external */
    sum += tls_external_weak;
    
    /* Use static TLS in function */
    use_static_tls();
    
    /* Force address taking for all variables */
    asm volatile("" : : 
        "r"(&tls_public_default),
        "r"(&tls_public_hidden),
        "r"(&tls_weak_var),
        "r"(&tls_common),
        "r"(&tls_external_var),
        "r"(&tls_external_array),
        "r"(&tls_external_weak)
    );
    
    return sum;
}

/* Another function with different access pattern */
void modify_tls_values(void) {
    tls_public_default *= 2;
    tls_public_hidden += 5;
    tls_weak_var -= 10;
    tls_common = tls_public_default + tls_public_hidden;
    
    /* Conditional logic using TLS */
    if (tls_public_default > 100) {
        tls_public_hidden = 0;
    }
    
    /* Loop with TLS dependency */
    for (int i = 0; i < tls_common % 10; i++) {
        tls_external_array[i] = i;
    }
}

/* Thread ID simulation using TLS */
__thread uintptr_t tls_thread_id __attribute__((used)) = 0;

void set_thread_id(uintptr_t id) {
    tls_thread_id = id;
}

uintptr_t get_thread_id(void) {
    return tls_thread_id;
}

/* Main function */
int main(void) {
    uint64_t checksum1, checksum2;
    
    /* Initialize TLS variables */
    tls_common = 50;
    set_thread_id((uintptr_t)main);
    
    /* First computation */
    checksum1 = compute_tls_checksum();
    printf("Initial TLS checksum: %llu\n", (unsigned long long)checksum1);
    
    /* Modify values */
    modify_tls_values();
    
    /* Second computation */
    checksum2 = compute_tls_checksum();
    printf("Modified TLS checksum: %llu\n", (unsigned long long)checksum2);
    
    /* Use thread ID */
    printf("Thread ID: %p\n", (void*)get_thread_id());
    
    /* Verify all TLS variables are unique */
    printf("TLS addresses:\n");
    printf("  tls_public_default: %p\n", &tls_public_default);
    printf("  tls_public_hidden: %p\n", &tls_public_hidden);
    printf("  tls_weak_var: %p\n", &tls_weak_var);
    printf("  tls_common: %p\n", &tls_common);
    printf("  tls_thread_id: %p\n", &tls_thread_id);
    
    return (checksum1 != checksum2) ? 0 : 1;
}
