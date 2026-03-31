/* tls_main.c - Main file with various TLS declarations */

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
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 999;
    tls_static_func++;
    /* Force address taken to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* Public function using TLS */
void use_public_tls(void) {
    tls_public_default++;
    tls_common += 2;
    tls_hidden--;
    
    /* Use external TLS */
    tls_external = tls_public_default + 1;
    
    /* Use weak TLS */
    if (&tls_weak != NULL) {
        tls_weak += 3;
    }
    
    /* Use function-static TLS */
    func_with_static_tls();
}

/* Another function using different TLS variables */
void use_more_tls(void) {
    /* Chain calculations through TLS variables */
    tls_external_hidden = tls_hidden * 2;
    tls_common = tls_external_hidden / 4;
    
    /* Simulate DLL import usage */
    int local = 0;
#ifdef _WIN32
    local = tls_dllimport;
#else
    /* For non-Windows, we'll just reference it */
    asm volatile("" : : "r"(&tls_dllimport));
#endif
    
    /* Use external weak if available */
    if (&tls_external_weak != NULL) {
        tls_external_weak = local + 1;
    }
}

/* Checksum function to ensure all TLS is used */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_common;
    
    if (&tls_weak != NULL) {
        sum += tls_weak;
    }
    
    sum += tls_external;
    sum += tls_external_hidden;
    
    if (&tls_external_weak != NULL) {
        sum += tls_external_weak;
    }
    
    return sum;
}

int main(void) {
    printf("Starting TLS attribute copy test...\n");
    
    /* Initialize TLS variables */
    tls_public_default = 1;
    tls_hidden = 2;
    tls_weak = 3;
    tls_common = 4;
    
    /* Use all TLS access patterns */
    use_public_tls();
    use_more_tls();
    
    /* Force multiple accesses to ensure instantiation */
    for (int i = 0; i < 3; i++) {
        tls_public_default *= 2;
        tls_hidden += i;
        use_public_tls();
    }
    
    /* Calculate and print checksum */
    uint32_t checksum = tls_checksum();
    printf("TLS checksum: %u\n", checksum);
    printf("TLS values: public=%d, hidden=%d, common=%d\n",
           tls_public_default, tls_hidden, tls_common);
    
    return 0;
}
