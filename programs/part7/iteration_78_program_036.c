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

/* DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak;
extern DLL_IMPORT __thread int tls_dllimport;

/* Function prototypes from other files */
void test_func1(void);
void test_func2(void);
void test_func3(void);

/* Static function with local TLS */
static void static_func(void) {
    /* TLS with function context */
    static __thread int tls_in_func = 500;
    tls_in_func++;
    
    /* Use all TLS variables to prevent optimization */
    volatile int sum = tls_public_default + tls_public_used + tls_weak + 
                      tls_common + tls_hidden + tls_in_func;
    (void)sum;
}

/* Force address taking without side effects */
#define FORCE_USE(var) asm volatile("" : : "r"(&(var)))

/* Initialize and use all TLS variables */
void init_tls_vars(void) {
    tls_public_default = 1;
    tls_public_used = 2;
    tls_weak = 3;
    tls_common = 4;  /* Initialize common variable */
    tls_hidden = 5;
    
    /* Force use of external variables */
    FORCE_USE(tls_external);
    FORCE_USE(tls_external_weak);
    FORCE_USE(tls_dllimport);
    
    static_func();
}

/* Calculate checksum of all TLS variables */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    /* Access all TLS variables */
    sum += tls_public_default;
    sum += tls_public_used;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    
    /* Try to access externals (may be 0 if not linked) */
    sum += tls_external;
    sum += tls_external_weak;
    sum += tls_dllimport;
    
    return sum;
}

int main(void) {
    /* Initialize TLS variables */
    init_tls_vars();
    
    /* Call test functions from other files */
    test_func1();
    test_func2();
    test_func3();
    
    /* Modify TLS variables */
    tls_public_default *= 2;
    tls_public_used += 10;
    tls_weak -= 5;
    tls_common = tls_checksum() % 100;
    tls_hidden ^= 0xFF;
    
    /* Call static function again */
    static_func();
    
    /* Final checksum */
    uint32_t final_sum = tls_checksum();
    printf("TLS checksum: %u\n", final_sum);
    
    /* Force all TLS addresses to be taken in main */
    FORCE_USE(tls_public_default);
    FORCE_USE(tls_public_used);
    FORCE_USE(tls_weak);
    FORCE_USE(tls_common);
    FORCE_USE(tls_hidden);
    
    return (int)(final_sum % 256);
}
