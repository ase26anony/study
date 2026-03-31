/* tls_main.c - Main file with various TLS declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize(0)
#endif

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((used, visibility("default"))) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_dllimport __attribute__((dllimport));

/* Function declarations */
void test_tls_access(void);
void test_tls_context(void);
void test_external_tls(void);
uint32_t compute_tls_checksum(void);

/* Prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(&(var)))

/* Static function with local TLS */
static void static_func_with_tls(void) {
    /* TLS with function context */
    static __thread int tls_in_static_func = 300;
    tls_in_static_func++;
    KEEP(tls_in_static_func);
}

/* Another static function with different TLS */
static void another_static_func(void) {
    __thread int tls_local_context = 500;
    tls_local_context = tls_public_default + 1;
    KEEP(tls_local_context);
}

void test_tls_access(void) {
    /* Access all TLS variables to prevent elimination */
    tls_public_default++;
    tls_hidden += 2;
    
    if (&tls_weak) {  /* Address check for weak symbol */
        tls_weak += 3;
    }
    
    tls_common = tls_public_default * 2;
    
    static_func_with_tls();
    another_static_func();
    
    /* Force address taking */
    KEEP(tls_public_default);
    KEEP(tls_hidden);
    KEEP(tls_weak);
    KEEP(tls_common);
}

void test_external_tls(void) {
    /* Access external TLS variables */
    if (&tls_external) {
        tls_external++;
    }
    
    if (&tls_external_weak) {
        tls_external_weak = tls_public_default + 10;
    }
    
    /* DLL import style access */
    if (&tls_dllimport) {
        volatile int val = tls_dllimport;
        (void)val;
    }
    
    KEEP(tls_external);
    KEEP(tls_external_weak);
    KEEP(tls_dllimport);
}

uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    /* Access all TLS variables in a way that can't be optimized out */
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_external;
    sum += tls_external_weak;
    
    /* Force computation to be used */
    asm volatile("" : "+r"(sum));
    return sum;
}

int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initialize common TLS */
    tls_common = 1234;
    
    /* Multiple accesses to ensure TLS instantiation */
    for (int i = 0; i < 3; i++) {
        test_tls_access();
        test_external_tls();
        
        /* Call static functions to establish their contexts */
        static_func_with_tls();
        another_static_func();
    }
    
    /* Compute and print checksum */
    uint32_t checksum = compute_tls_checksum();
    printf("TLS checksum: %u\n", checksum);
    
    /* Verify values */
    printf("tls_public_default: %d\n", tls_public_default);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_common: %d\n", tls_common);
    
    return 0;
}
