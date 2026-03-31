/* Main file with various TLS declarations */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;
__thread int tls_public_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* External declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_dllimport __attribute__((dllimport));

/* Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 300;
    tls_static_func++;
    /* Force address taken to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* TLS with DLL import attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate DLL import on non-Windows */
__thread int tls_imported __attribute__((dllimport));
#endif

/* Function to use all TLS variables */
uint32_t use_tls_variables(void) {
    uint32_t checksum = 0;
    
    /* Access public TLS */
    checksum += tls_public_default;
    tls_public_default++;
    
    checksum += tls_public_hidden;
    tls_public_hidden += 2;
    
    /* Access weak TLS */
    if (&tls_weak) {
        checksum += tls_weak;
        tls_weak += 3;
    }
    
    /* Access common TLS */
    checksum += tls_common;
    tls_common += 4;
    
    /* Access external TLS */
    checksum += tls_external;
    
    /* Access external weak TLS */
    if (&tls_external_weak) {
        checksum += tls_external_weak;
    }
    
    /* Access static function TLS */
    func_with_static_tls();
    
    /* Prevent optimization of all variables */
    asm volatile("" : : 
        "r"(&tls_public_default),
        "r"(&tls_public_hidden),
        "r"(&tls_weak),
        "r"(&tls_common),
        "r"(&tls_external),
        "r"(&tls_external_weak),
        "r"(&tls_dllimport),
        "r"(&tls_imported)
    );
    
    return checksum;
}

/* Another function with different access pattern */
void modify_tls_variables(void) {
    tls_public_default *= 2;
    tls_public_hidden /= 2;
    tls_weak ^= 0x55;
    tls_common |= 0xAA;
}

int main(void) {
    uint32_t checksum1, checksum2;
    
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initial use */
    checksum1 = use_tls_variables();
    printf("Initial checksum: %u\n", checksum1);
    
    /* Modify and use again */
    modify_tls_variables();
    checksum2 = use_tls_variables();
    printf("Modified checksum: %u\n", checksum2);
    
    /* Force context usage */
    func_with_static_tls();
    
    return 0;
}
