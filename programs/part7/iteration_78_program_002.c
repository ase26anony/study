/* tls_main.c - Main file with various TLS declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#pragma GCC optimize("O0")

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int tls_external_default;
extern __thread int tls_external_hidden;
extern __thread int tls_external_weak __attribute__((weak));

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
extern __thread int tls_dllimport __attribute__((dllimport));
#else
/* Simulate with visibility on non-Windows */
extern __thread int tls_dllimport __attribute__((visibility("default")));
#endif

/* Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 300;
    tls_static_func++;
    /* Force address taken to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* Public function using TLS */
void use_tls_variables(void) {
    /* Access all TLS variables to ensure they're used */
    tls_public_default++;
    tls_public_hidden += 2;
    
    if (tls_weak_var) {
        tls_weak_var *= 2;
    }
    
    tls_common = tls_public_default + tls_public_hidden;
    
    /* Access externals */
    tls_external_default++;
    tls_external_hidden += 3;
    
    if (&tls_external_weak) {
        tls_external_weak--;
    }
    
    if (&tls_dllimport) {
        tls_dllimport += 100;
    }
    
    func_with_static_tls();
}

/* Another function with different access pattern */
void modify_tls_variables(int multiplier) {
    tls_public_default *= multiplier;
    tls_public_hidden /= (multiplier > 0 ? multiplier : 1);
    tls_weak_var += multiplier;
    tls_common -= multiplier;
}

/* Checksum function to verify all TLS variables are active */
uint64_t tls_checksum(void) {
    uint64_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_public_hidden;
    sum += tls_weak_var;
    sum += tls_common;
    sum += tls_external_default;
    sum += tls_external_hidden;
    
    if (&tls_external_weak) {
        sum += tls_external_weak;
    }
    
    if (&tls_dllimport) {
        sum += tls_dllimport;
    }
    
    return sum;
}

int main(void) {
    printf("Initial TLS checksum: %llu\n", 
           (unsigned long long)tls_checksum());
    
    /* First usage */
    use_tls_variables();
    printf("After first use: %llu\n", 
           (unsigned long long)tls_checksum());
    
    /* Modify with different patterns */
    modify_tls_variables(2);
    printf("After modification: %llu\n", 
           (unsigned long long)tls_checksum());
    
    /* Use again */
    use_tls_variables();
    printf("Final checksum: %llu\n", 
           (unsigned long long)tls_checksum());
    
    /* Force all TLS addresses to be taken */
    void* addresses[] = {
        &tls_public_default,
        &tls_public_hidden,
        &tls_weak_var,
        &tls_common,
        &tls_external_default,
        &tls_external_hidden,
        &tls_external_weak,
        &tls_dllimport
    };
    
    /* Use addresses in asm to prevent optimization */
    asm volatile("" : : "r"(addresses) : "memory");
    
    return 0;
}
