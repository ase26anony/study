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

/* DLL import simulation */
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
#else
    #define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int tls_external;
extern __thread int tls_external_hidden;
extern DLL_IMPORT __thread int tls_dllimport;

/* Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 300;
    tls_static_func++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* Public TLS used attribute */
__thread uint64_t tls_used_counter __attribute__((used)) = 0;

/* Function that uses all TLS variables */
void update_tls_values(void) {
    tls_public_default++;
    tls_hidden += 2;
    
    /* Weak TLS - may be overridden */
    if (&tls_weak != NULL) {
        tls_weak += 3;
    }
    
    tls_common += 4;
    tls_external += 5;
    tls_external_hidden += 6;
    
    /* DLL import - simulate access */
    #ifdef _WIN32
        tls_dllimport += 7;
    #endif
    
    tls_used_counter++;
    func_with_static_tls();
}

/* Another function with different access pattern */
void checksum_tls_values(void) {
    uint64_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_external;
    sum += tls_external_hidden;
    #ifdef _WIN32
        sum += tls_dllimport;
    #endif
    sum += tls_used_counter;
    
    /* Print checksum to ensure variables are used */
    printf("TLS checksum: %llu\n", (unsigned long long)sum);
    
    /* Force compiler to keep all variables */
    asm volatile("" : : 
        "r"(tls_public_default), "r"(tls_hidden), "r"(tls_weak),
        "r"(tls_common), "r"(tls_external), "r"(tls_external_hidden),
        "r"(tls_used_counter)
    );
}

/* Thread ID simulation using TLS */
__thread int thread_specific_id __attribute__((visibility("default")));

int main(void) {
    /* Initialize thread-specific ID */
    thread_specific_id = 1;
    
    /* Multiple accesses to trigger emulated TLS initialization */
    for (int i = 0; i < 10; i++) {
        update_tls_values();
        checksum_tls_values();
    }
    
    /* Access via pointer to ensure address is taken */
    __thread int* tls_ptr __attribute__((unused));
    tls_ptr = &tls_public_default;
    
    /* Complex expression using TLS */
    tls_common = tls_public_default * 2 + tls_hidden;
    
    checksum_tls_values();
    
    return 0;
}
