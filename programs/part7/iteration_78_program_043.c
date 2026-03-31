/* Main file with various TLS declarations */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")  /* Prevent optimization of TLS variables */
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 0;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* DLL import simulation (for attribute testing) */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Extern declarations (defined in another file) */
extern __thread int tls_extern_var;
extern __thread int tls_extern_weak __attribute__((weak));
extern __thread char tls_extern_array[16];

/* Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 777;
    static __thread int tls_func_common;
    
    tls_func_static++;
    tls_func_common = tls_func_static * 2;
    
    /* Force usage to prevent elimination */
    asm volatile("" : : "r"(&tls_func_static), "r"(&tls_func_common));
}

/* Public function using TLS */
void use_public_tls(void) {
    tls_public_default++;
    tls_public_hidden += 2;
    
    /* Force address taking */
    asm volatile("" : : "r"(&tls_public_default), "r"(&tls_public_hidden));
}

/* Function using weak TLS */
void use_weak_tls(void) {
    if (&tls_weak_var != NULL) {
        tls_weak_var++;
    }
    
    /* Also use extern weak */
    if (&tls_extern_weak != NULL) {
        tls_extern_weak--;
    }
}

/* Function using common TLS */
void use_common_tls(void) {
    tls_common = (tls_common + 1) % 100;
    
    /* Use extern array */
    for (int i = 0; i < 16; i++) {
        tls_extern_array[i] = (char)(tls_common + i);
    }
}

/* Checksum calculation across all TLS variables */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_public_hidden;
    sum += tls_weak_var;
    sum += tls_common;
    sum += tls_extern_var;
    
    for (int i = 0; i < 16; i++) {
        sum += (uint8_t)tls_extern_array[i];
    }
    
    func_with_static_tls();
    
    return sum;
}

int main(void) {
    /* Initialize TLS variables */
    tls_public_default = 1;
    tls_public_hidden = 2;
    tls_weak_var = 3;
    tls_common = 4;
    
    /* Use all TLS variables through various functions */
    use_public_tls();
    use_weak_tls();
    use_common_tls();
    
    /* Call function multiple times to ensure TLS persistence */
    for (int i = 0; i < 3; i++) {
        func_with_static_tls();
        use_public_tls();
    }
    
    /* Calculate and print checksum */
    uint32_t checksum = tls_checksum();
    printf("TLS checksum: %u\n", checksum);
    
    /* Force all TLS addresses to be taken (prevents elimination) */
    void* addresses[] = {
        &tls_public_default,
        &tls_public_hidden,
        &tls_weak_var,
        &tls_common,
        &tls_extern_var,
        &tls_extern_weak,
        tls_extern_array
    };
    
    asm volatile("" : : "r"(addresses));
    
    return 0;
}
