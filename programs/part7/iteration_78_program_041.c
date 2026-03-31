/* File 1: Main program with various TLS variables */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")  /* Prevent optimization of TLS variables */
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

/* Common linkage (tentative definition) - no initializer */
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
extern __thread int tls_external_hidden;

/* Function prototypes */
void test_func1(void);
void test_func2(void);
void test_func3(void);

/* Static function with local TLS */
static void static_func(void) {
    /* TLS in static function context */
    static __thread int tls_static_context = 300;
    tls_static_context++;
    
    /* Force usage to prevent optimization */
    asm volatile("" : : "r"(&tls_static_context));
}

/* Use all TLS variables to prevent elimination */
void use_tls_variables(void) {
    /* Read and modify all TLS variables */
    tls_public_default += 1;
    tls_hidden *= 2;
    tls_weak -= 5;
    tls_common = tls_public_default + tls_hidden;
    
    /* Access external TLS */
    int ext_val = tls_external;
    tls_external = ext_val + 10;
    
    /* Access weak external */
    tls_external_weak += 3;
    
    /* Access hidden external */
    tls_external_hidden *= 1;
    
    /* Call static function */
    static_func();
    
    /* Force address-taking without side effects */
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

/* Thread-specific counter in TLS */
__thread uint64_t tls_thread_counter = 0;

/* Complex TLS usage pattern */
void increment_counters(void) {
    for (int i = 0; i < 10; i++) {
        tls_thread_counter++;
        tls_public_default++;
        tls_hidden--;
    }
    
    /* Conditional logic based on TLS */
    if (tls_thread_counter > 5) {
        tls_weak = tls_thread_counter;
    }
    
    /* Store thread-specific data */
    __thread void* tls_pointer = &tls_thread_counter;
    asm volatile("" : : "r"(tls_pointer));
}

int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initialize TLS variables */
    tls_public_default = 1;
    tls_hidden = 2;
    tls_weak = 3;
    tls_common = 4;
    tls_thread_counter = 0;
    
    /* Use TLS variables multiple times */
    for (int i = 0; i < 3; i++) {
        use_tls_variables();
        increment_counters();
        test_func1();
        test_func2();
        test_func3();
        static_func();
    }
    
    /* Calculate checksum of all TLS values */
    uint64_t checksum = 0;
    checksum += tls_public_default;
    checksum += tls_hidden;
    checksum += tls_weak;
    checksum += tls_common;
    checksum += tls_thread_counter;
    
    /* Access external variables for checksum */
    checksum += tls_external;
    checksum += tls_external_weak;
    checksum += tls_external_hidden;
    
    printf("TLS checksum: %llu\n", (unsigned long long)checksum);
    
    /* Verify values are non-zero (ensuring they're used) */
    if (checksum == 0) {
        return 1; /* Error - all TLS variables optimized out */
    }
    
    return 0;
}
