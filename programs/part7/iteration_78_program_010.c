/* tls_main.c - Main file with various TLS variables */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#if defined(__x86_64__)
#pragma GCC target("tls,emulated")
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((used, visibility("default")));
__thread int tls_public_hidden __attribute__((used, visibility("hidden")));

/* Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dll_import;
#else
__thread int tls_dll_import __attribute__((dllimport));
#endif

/* External declarations (defined in tls_aux.c) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 100;
    tls_func_static++;
    /* Prevent optimization */
    asm volatile("" : : "r"(&tls_func_static));
}

/* Public function using TLS */
void use_public_tls(void) {
    tls_public_default++;
    tls_public_hidden--;
    
    /* Force address taking */
    volatile int *addr1 = &tls_public_default;
    volatile int *addr2 = &tls_public_hidden;
    (void)addr1;
    (void)addr2;
}

/* Function using weak TLS */
void use_weak_tls(void) {
    if (&tls_weak_var != NULL) {
        tls_weak_var *= 2;
    }
    
    /* Also use external weak */
    if (&tls_external_weak != NULL) {
        tls_external_weak += 3;
    }
}

/* Function using common TLS */
void use_common_tls(void) {
    tls_common = (tls_common == 0) ? 1 : tls_common * 2;
}

/* Function using external TLS */
void use_external_tls(void) {
    tls_external += 7;
    tls_external_hidden -= 2;
}

/* Checksum function to ensure all TLS is used */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_public_hidden;
    sum += tls_weak_var;
    sum += tls_common;
    sum += tls_external;
    sum += tls_external_weak;
    sum += tls_external_hidden;
    
    /* Force DLL import usage if available */
    if (&tls_dll_import != NULL) {
        sum += tls_dll_import;
    }
    
    func_with_static_tls();
    
    return sum;
}

int main(void) {
    /* Initialize TLS variables */
    tls_public_default = 1;
    tls_public_hidden = 2;
    tls_common = 3;
    
    /* Use all TLS variables through various functions */
    use_public_tls();
    use_weak_tls();
    use_common_tls();
    use_external_tls();
    
    /* Multiple calls to ensure TLS is active */
    for (int i = 0; i < 3; i++) {
        use_public_tls();
        use_weak_tls();
    }
    
    /* Calculate and print checksum */
    uint32_t checksum = tls_checksum();
    printf("TLS checksum: %u\n", checksum);
    
    /* Force all TLS addresses to be taken */
    volatile void *addrs[] = {
        &tls_public_default,
        &tls_public_hidden,
        &tls_weak_var,
        &tls_common,
        &tls_external,
        &tls_external_weak,
        &tls_external_hidden,
        &tls_dll_import
    };
    (void)addrs;
    
    return 0;
}
