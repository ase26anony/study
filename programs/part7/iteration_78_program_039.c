/* tls_main.c - Main file with various TLS variables */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* Public TLS with explicit visibility and used attribute */
__thread int tls_public_used __attribute__((used, visibility("default"))) = 42;

/* Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 100;

/* Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_dllimport __attribute__((dllimport));

/* Function prototypes */
void test_func1(void);
void test_func2(void);
void test_func3(void);
uint32_t compute_checksum(void);

/* Static function with local TLS */
static void static_function(void) {
    /* TLS with function context */
    static __thread int tls_in_function = 999;
    tls_in_function++;
    
    /* Prevent elimination */
    asm volatile("" : : "r"(&tls_in_function));
}

/* Another static function with different TLS */
static void another_static(void) {
    __thread int tls_local_static = 1234;
    tls_local_static += tls_public_used;
    
    /* Force address taking */
    volatile int *ptr = &tls_local_static;
    (void)ptr;
}

int main(void) {
    uint32_t checksum;
    
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Access all TLS variables to ensure instantiation */
    tls_public_used = 1;
    tls_weak_var = 2;
    tls_common = 3;
    tls_hidden = 4;
    
    /* Access extern TLS (will be resolved from tls_aux.c) */
    tls_external = 5;
    
    /* Call functions that use TLS */
    test_func1();
    test_func2();
    test_func3();
    
    /* Call static functions */
    static_function();
    another_static();
    
    /* Compute and print checksum */
    checksum = compute_checksum();
    printf("TLS checksum: %u\n", checksum);
    
    /* Additional accesses to ensure all paths are taken */
    if (tls_external_weak) {
        tls_external_weak++;
    }
    
    /* Force DLL import attribute usage */
#ifdef _WIN32
    if (tls_dllimport) {
        printf("DLL import TLS accessed\n");
    }
#endif
    
    return 0;
}

/* Function that uses TLS variables */
void test_func1(void) {
    tls_public_used++;
    tls_common += 10;
    
    /* Force context usage */
    DECL_CONTEXT: __attribute__((unused));
}

/* Another function using different TLS vars */
void test_func2(void) {
    tls_hidden *= 2;
    tls_weak_var -= 5;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&tls_hidden), "r"(&tls_weak_var));
}

/* Compute checksum of all TLS variables */
uint32_t compute_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_used;
    sum += tls_weak_var;
    sum += tls_common;
    sum += tls_hidden;
    sum += tls_external;
    
    /* Access through pointer to force address computation */
    volatile __thread int *vars[] = {
        &tls_public_used,
        &tls_weak_var,
        &tls_common,
        &tls_hidden,
        &tls_external,
    };
    
    for (int i = 0; i < 5; i++) {
        sum += *vars[i];
    }
    
    return sum;
}
