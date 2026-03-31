/* Main file with varied TLS declarations and usage */

/* Force emulated TLS even if native is available */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* Public TLS with explicit default visibility */
__attribute__((visibility("default")))
__thread int tls_public_default = 42;

/* Public TLS with used attribute */
__attribute__((used))
__thread int tls_public_used = 100;

/* Weak TLS definition */
__attribute__((weak))
__thread int tls_weak = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* Hidden visibility TLS */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 300;

/* DLL import simulation (for attribute testing) */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak;
extern __thread int tls_external_hidden;

/* Function prototypes */
void test_func1(void);
void test_func2(void);
void test_func3(void);

/* Static function with local TLS */
static void static_func(void) {
    /* TLS with function context */
    static __thread int tls_in_func = 999;
    tls_in_func++;
    
    /* Use all TLS variables to prevent optimization */
    tls_public_default += tls_in_func;
    tls_hidden -= tls_in_func;
}

/* Force address taking without side effects */
#define FORCE_USE(var) asm volatile("" : : "r"(&(var)))

/* Initialize and use all TLS variables */
void init_tls(void) {
    /* Initialize common TLS */
    tls_common = 500;
    
    /* Modify all TLS variables */
    tls_public_default++;
    tls_public_used *= 2;
    tls_weak += 3;
    tls_hidden -= 4;
    
    /* Access external TLS */
    tls_external = 600;
    tls_external_weak = 700;
    tls_external_hidden = 800;
    
    /* Force use of all addresses */
    FORCE_USE(tls_public_default);
    FORCE_USE(tls_public_used);
    FORCE_USE(tls_weak);
    FORCE_USE(tls_common);
    FORCE_USE(tls_hidden);
    FORCE_USE(tls_external);
    FORCE_USE(tls_external_weak);
    FORCE_USE(tls_external_hidden);
    
    /* Call static function */
    static_func();
}

/* Calculate checksum of all TLS values */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_public_used;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    sum += tls_external;
    sum += tls_external_weak;
    sum += tls_external_hidden;
    
    return sum;
}

int main(void) {
    printf("Initializing TLS variables...\n");
    
    /* Initialize and use TLS */
    init_tls();
    
    /* Call test functions from different files */
    test_func1();
    test_func2();
    test_func3();
    
    /* Calculate and print checksum */
    uint32_t checksum = tls_checksum();
    printf("TLS checksum: %u\n", checksum);
    
    /* Verify values */
    printf("tls_public_default: %d\n", tls_public_default);
    printf("tls_public_used: %d\n", tls_public_used);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_common: %d\n", tls_common);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_external: %d\n", tls_external);
    printf("tls_external_weak: %d\n", tls_external_weak);
    printf("tls_external_hidden: %d\n", tls_external_hidden);
    
    return 0;
}
