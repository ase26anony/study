/* Main file with various TLS declarations */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC target("tls,emulated")
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* Static function with local TLS */
static void static_func(void) {
    static __thread int tls_local_static = 300;
    tls_local_static++;
    
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_local_static));
}

/* Public function using TLS */
void use_tls_variables(void) {
    /* Access all TLS variables */
    tls_public_default++;
    tls_hidden += 2;
    
    if (&tls_weak) {  /* Ensure weak symbol is referenced */
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
    
    /* Prevent elimination */
    volatile int sum = tls_public_default + tls_hidden + tls_weak + 
                      tls_common + tls_external + tls_external_weak + 
                      tls_external_hidden;
    (void)sum;
}

/* Another function with different TLS usage pattern */
void modify_tls_values(int multiplier) {
    tls_public_default *= multiplier;
    tls_hidden *= multiplier + 1;
    
    if (&tls_weak) {
        tls_weak *= multiplier + 2;
    }
    
    tls_common *= multiplier + 3;
    
    /* Force all addresses to be taken */
    asm volatile("" : : 
        "r"(&tls_public_default),
        "r"(&tls_hidden),
        "r"(&tls_weak),
        "r"(&tls_common),
        "r"(&tls_external),
        "r"(&tls_external_weak),
        "r"(&tls_external_hidden)
    );
}

int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initial modifications */
    use_tls_variables();
    modify_tls_values(2);
    
    /* Second round */
    use_tls_variables();
    modify_tls_values(3);
    
    /* Calculate checksum */
    uintptr_t checksum = 0;
    checksum += (uintptr_t)&tls_public_default;
    checksum += (uintptr_t)&tls_hidden;
    checksum += (uintptr_t)&tls_weak;
    checksum += (uintptr_t)&tls_common;
    checksum += (uintptr_t)&tls_external;
    checksum += (uintptr_t)&tls_external_weak;
    checksum += (uintptr_t)&tls_external_hidden;
    
    /* Access values */
    checksum += tls_public_default;
    checksum += tls_hidden;
    checksum += tls_weak;
    checksum += tls_common;
    checksum += tls_external;
    checksum += tls_external_weak;
    checksum += tls_external_hidden;
    
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    
    return 0;
}
