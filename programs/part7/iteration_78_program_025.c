/* tls_main.c - Main file with various TLS variable declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize(0)
#endif

/* Public TLS variable with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;

/* Hidden visibility TLS variable */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage TLS (tentative definition) */
__thread int tls_common;

/* DLL import simulation */
#ifdef _WIN32
__thread int tls_dllimport __attribute__((dllimport));
#else
/* Simulate with external declaration for non-Windows */
extern __thread int tls_dllimport;
#endif

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* Static function with local TLS variable */
static void static_func(void) {
    /* TLS variable with function context */
    static __thread int tls_in_func = 300;
    tls_in_func++;
    
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_in_func));
}

/* Public function using TLS */
void use_tls_variables(void) {
    /* Access all TLS variables to ensure they're used */
    tls_public_default++;
    tls_hidden += 2;
    
    if (&tls_weak) {  /* Check if weak symbol exists */
        tls_weak += 3;
    }
    
    tls_common += 4;
    
    /* External TLS access */
    tls_external += 5;
    
    if (&tls_external_weak) {
        tls_external_weak += 6;
    }
    
    tls_external_hidden += 7;
    
    /* Call static function */
    static_func();
    
    /* Prevent dead code elimination */
    volatile int sum = tls_public_default + tls_hidden + tls_common;
    asm volatile("" : : "r"(sum));
}

/* Another function with different TLS usage pattern */
void modify_tls_variables(void) {
    /* Different modification pattern */
    tls_public_default *= 2;
    tls_hidden /= 2;
    tls_common = tls_public_default + tls_hidden;
    
    /* Chain external TLS modifications */
    tls_external = tls_external_weak + tls_external_hidden;
    
    /* Force compiler to keep variables */
    asm volatile("" : : 
        "r"(tls_public_default), 
        "r"(tls_hidden),
        "r"(tls_common),
        "r"(tls_external)
    );
}

/* Thread ID simulation using TLS */
__thread uintptr_t thread_specific_id = 0;

void set_thread_id(uintptr_t id) {
    thread_specific_id = id;
}

uintptr_t get_thread_id(void) {
    return thread_specific_id;
}

/* Checksum function to verify all TLS variables are active */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_hidden;
    
    if (&tls_weak) {
        sum += tls_weak;
    }
    
    sum += tls_common;
    sum += tls_external;
    
    if (&tls_external_weak) {
        sum += tls_external_weak;
    }
    
    sum += tls_external_hidden;
    sum += (uint32_t)thread_specific_id;
    
    return sum;
}

int main(void) {
    /* Initialize thread ID */
    set_thread_id(0x12345678);
    
    /* Multiple accesses to ensure TLS instantiation */
    for (int i = 0; i < 3; i++) {
        use_tls_variables();
        modify_tls_variables();
    }
    
    /* Print checksum to verify all TLS variables are active */
    uint32_t checksum = tls_checksum();
    printf("TLS checksum: 0x%08x\n", checksum);
    
    /* Additional verification */
    printf("Thread ID: 0x%08lx\n", (unsigned long)get_thread_id());
    printf("Public TLS: %d\n", tls_public_default);
    printf("Hidden TLS: %d\n", tls_hidden);
    printf("Common TLS: %d\n", tls_common);
    
    return 0;
}
