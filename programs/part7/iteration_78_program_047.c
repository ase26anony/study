/* tls_main.c - Main file with various TLS declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* External TLS declaration (defined in tls_aux.c) */
extern __thread int tls_external;

/* DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with weak external */
extern __thread int tls_dllimport __attribute__((weak));
#endif

/* Function prototypes */
void test_tls_access(void);
void test_static_tls(void);
uint32_t compute_tls_checksum(void);

/* Static function with local TLS */
static void static_func_with_tls(void) {
    /* TLS with function context */
    static __thread int local_func_tls = 999;
    local_func_tls++;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&local_func_tls));
}

/* Global function using TLS */
void test_tls_access(void) {
    /* Access all TLS variables */
    tls_public_default++;
    tls_hidden += 2;
    
    if (&tls_weak) {  /* Ensure address is taken */
        tls_weak += 3;
    }
    
    tls_common += 4;
    tls_external += 5;
    
    #ifndef _WIN32
    if (&tls_dllimport) {
        tls_dllimport += 6;
    }
    #endif
    
    /* Call static function */
    static_func_with_tls();
    
    /* Prevent elimination */
    asm volatile("" : : 
        "r"(&tls_public_default), 
        "r"(&tls_hidden),
        "r"(&tls_common),
        "r"(&tls_external)
    );
}

/* Another test function */
void test_static_tls(void) {
    /* Static TLS inside function */
    static __thread uint64_t static_local_tls = 0xDEADBEEF;
    static_local_tls ^= 0x12345678;
    
    /* More complex usage pattern */
    tls_public_default = (tls_public_default * 3) / 2;
    tls_hidden = tls_hidden | 0xFF00;
    
    /* Conditional based on TLS */
    if (tls_common > 1000) {
        tls_common = 0;
    }
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&static_local_tls));
}

/* Compute checksum of all TLS values */
uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_external;
    
    #ifndef _WIN32
    if (&tls_dllimport) {
        sum += tls_dllimport;
    }
    #endif
    
    return sum;
}

int main(void) {
    printf("Starting TLS attribute coverage test...\n");
    
    /* Initial access to instantiate TLS */
    test_tls_access();
    test_static_tls();
    
    /* Access from multiple points */
    for (int i = 0; i < 3; i++) {
        tls_public_default += i;
        tls_hidden -= i;
        test_tls_access();
    }
    
    /* Final checksum */
    uint32_t checksum = compute_tls_checksum();
    printf("TLS checksum: %u\n", checksum);
    printf("TLS variable addresses:\n");
    printf("  tls_public_default: %p\n", (void*)&tls_public_default);
    printf("  tls_hidden: %p\n", (void*)&tls_hidden);
    printf("  tls_weak: %p\n", (void*)&tls_weak);
    printf("  tls_common: %p\n", (void*)&tls_common);
    printf("  tls_external: %p\n", (void*)&tls_external);
    
    return 0;
}
