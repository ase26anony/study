/* Main file with various TLS variable declarations */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#if defined(__x86_64__) && !defined(__ILP32__)
#pragma GCC target("general-regs-only")
#endif

/* Public TLS with explicit default visibility */
__attribute__((visibility("default")))
__thread int tls_public_default = 42;

/* Weak TLS definition */
__attribute__((weak))
__thread int tls_weak_var = 100;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* Hidden visibility */
__attribute__((visibility("hidden")))
__thread void* tls_hidden_ptr = (void*)0x1234;

/* Used attribute to ensure preservation */
__attribute__((used))
__thread uint64_t tls_used_counter = 0;

/* External declarations (defined in another file) */
extern __thread int tls_external_var;
extern __thread char tls_external_array[16];
extern __thread double tls_external_double;

/* DLL import simulation (for attribute testing) */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Simulated DLL import TLS */
extern DLL_IMPORT __thread int tls_dllimport_var;

/* Function declarations */
void test_func1(void);
void test_func2(void);
uint64_t compute_checksum(void);

/* Static function with local TLS */
static void static_function(void) {
    /* TLS in function scope with different context */
    static __thread int tls_local_static = 777;
    tls_local_static++;
    
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_local_static));
}

/* Thread ID simulation */
__thread int thread_simulated_id = 0;

int main(void) {
    /* Access and modify all TLS variables */
    thread_simulated_id = 1;
    
    tls_public_default *= 2;
    tls_weak_var += 10;
    tls_common = 999;
    tls_hidden_ptr = (void*)((uintptr_t)tls_hidden_ptr + 1);
    tls_used_counter++;
    
    /* Access external TLS */
    tls_external_var = 123;
    tls_external_array[0] = 'A';
    tls_external_double = 3.14159;
    
    /* Call functions that use TLS */
    test_func1();
    test_func2();
    static_function();
    
    /* Compute and print checksum */
    uint64_t checksum = compute_checksum();
    printf("TLS Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Force all TLS addresses to be taken */
    void* addresses[] = {
        &tls_public_default,
        &tls_weak_var,
        &tls_common,
        &tls_hidden_ptr,
        &tls_used_counter,
        &tls_external_var,
        &tls_external_array[0],
        &tls_external_double,
        &thread_simulated_id
    };
    
    asm volatile("" : : "r"(addresses) : "memory");
    
    return 0;
}

/* Compute checksum across all TLS variables */
uint64_t compute_checksum(void) {
    uint64_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_weak_var;
    sum += tls_common;
    sum += (uintptr_t)tls_hidden_ptr;
    sum += tls_used_counter;
    sum += tls_external_var;
    
    for (int i = 0; i < 16; i++) {
        sum += tls_external_array[i];
    }
    
    /* Cast double to uint64_t for checksum */
    union { double d; uint64_t u; } conv;
    conv.d = tls_external_double;
    sum += conv.u;
    
    sum += thread_simulated_id;
    
    return sum;
}
