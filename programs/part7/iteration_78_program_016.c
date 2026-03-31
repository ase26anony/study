/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* Public TLS with explicit visibility */
__thread int tls_public __attribute__((visibility("default"), used)) = 42;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 100;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__thread int tls_dllimport __attribute__((dllimport));
#else
/* Simulate with dllimport-like attribute if supported */
__thread int tls_dllimport __attribute__((weak, visibility("default")));
#endif

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* TLS pointer with complex usage */
__thread void* tls_pointer __attribute__((used));

/* TLS in different context (static function) */
static void use_static_context_tls(void) {
    /* TLS with function context */
    static __thread int tls_in_function = 0;
    tls_in_function++;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&tls_in_function));
}

/* ===== FUNCTIONS THAT USE TLS ===== */

/* Function that modifies all TLS variables */
void modify_tls_vars(int increment) {
    tls_public += increment;
    tls_weak += increment * 2;
    tls_common += increment * 3;
    tls_hidden += increment * 4;
    
    /* Use the static context TLS */
    use_static_context_tls();
    
    /* Store address in TLS pointer */
    tls_pointer = (void*)(uintptr_t)(tls_public + tls_weak);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&tls_public), "r"(&tls_weak), "r"(&tls_common), 
                               "r"(&tls_hidden), "r"(&tls_pointer));
}

/* Function that uses external TLS */
int use_external_tls(void) {
    int sum = tls_external + tls_external_weak + tls_external_hidden;
    
    /* Force address-taking to prevent optimization */
    asm volatile("" : : "r"(&tls_external), "r"(&tls_external_weak), 
                               "r"(&tls_external_hidden));
    
    return sum;
}

/* Checksum function to verify TLS values */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    sum += (uintptr_t)tls_pointer & 0xFFFFFFFF;
    sum += tls_external;
    sum += tls_external_weak;
    sum += tls_external_hidden;
    
    return sum;
}

/* Main function with extensive TLS usage */
int main(void) {
    printf("Starting emulated TLS test...\n");
    
    /* Initial modifications */
    modify_tls_vars(1);
    
    /* Use external TLS */
    int ext_sum = use_external_tls();
    printf("External TLS sum: %d\n", ext_sum);
    
    /* More modifications in different patterns */
    for (int i = 0; i < 5; i++) {
        modify_tls_vars(i);
        use_static_context_tls();
    }
    
    /* Final checksum */
    uint32_t final_sum = tls_checksum();
    printf("Final TLS checksum: %u\n", final_sum);
    
    /* Force all TLS addresses to be taken */
    asm volatile("" : : 
        "r"(&tls_public), "r"(&tls_weak), "r"(&tls_common),
        "r"(&tls_hidden), "r"(&tls_dllimport), "r"(&tls_pointer)
    );
    
    return 0;
}
