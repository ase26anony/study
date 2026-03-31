/* Main file with various TLS declarations and usage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* Public TLS with explicit visibility */
__attribute__((visibility("default"), used))
__thread int tls_public_default = 42;

/* Hidden visibility TLS */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 100;

/* Weak TLS definition */
__attribute__((weak))
__thread int tls_weak = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak;
extern __thread int tls_external_common;

/* Function prototypes */
void test_func1(void);
void test_func2(void);
void test_func3(void);
uint32_t compute_checksum(void);

/* Static function with local TLS */
static void static_func(void) {
    /* TLS in static function context */
    static __thread int tls_static_func = 300;
    tls_static_func++;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* Another static function with different TLS */
static void another_static_func(void) {
    __attribute__((used))
    __thread int tls_another_static = 400;
    
    tls_another_static += tls_public_default;
    asm volatile("" : : "r"(&tls_another_static));
}

/* Global function using TLS */
void test_func1(void) {
    /* Access various TLS variables */
    tls_public_default++;
    tls_hidden += 2;
    
    if (tls_weak == 200) {
        tls_weak = 201;
    }
    
    tls_common = tls_public_default + tls_hidden;
    
    /* Use external TLS */
    tls_external = tls_common + 1;
    
    static_func();
    another_static_func();
}

/* Function that uses weak external TLS */
void test_func2(void) {
    /* Access weak external TLS */
    if (tls_external_weak == 0) {
        tls_external_weak = 999;
    }
    
    tls_external_common = tls_external_weak * 2;
    
    /* Force address taking */
    asm volatile("" : : 
        "r"(&tls_external_weak), 
        "r"(&tls_external_common)
    );
}

/* Compute checksum of all TLS variables */
uint32_t compute_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_external;
    sum += tls_external_weak;
    sum += tls_external_common;
    
    /* Force all addresses to be taken */
    asm volatile("" : : 
        "r"(&tls_public_default),
        "r"(&tls_hidden),
        "r"(&tls_weak),
        "r"(&tls_common),
        "r"(&tls_external),
        "r"(&tls_external_weak),
        "r"(&tls_external_common)
    );
    
    return sum;
}

int main(void) {
    printf("Starting emulated TLS test...\n");
    
    /* Initialize TLS variables */
    tls_public_default = 1;
    tls_hidden = 2;
    tls_weak = 3;
    tls_common = 4;
    
    /* Call test functions multiple times */
    for (int i = 0; i < 3; i++) {
        test_func1();
        test_func2();
        test_func3();
        static_func();
        another_static_func();
    }
    
    /* Compute and print checksum */
    uint32_t checksum = compute_checksum();
    printf("TLS checksum: %u\n", checksum);
    
    /* Verify values */
    printf("tls_public_default: %d\n", tls_public_default);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_common: %d\n", tls_common);
    printf("tls_external: %d\n", tls_external);
    printf("tls_external_weak: %d\n", tls_external_weak);
    printf("tls_external_common: %d\n", tls_external_common);
    
    return 0;
}
