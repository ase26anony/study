/* test_emutls_coverage.c
 * 
 * This program creates multiple thread-local variables with different
 * attributes to trigger the property copying in GCC's emutls_decl function.
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 -fPIC test_emutls_coverage.c -o test_emutls
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is not available */
#ifndef __HAVE_TLS
#define USE_EMULATED_TLS 1
#endif

/* Global volatile array to prevent optimization */
volatile uintptr_t tls_addresses[10];

/* Helper function marked noinline to ensure TLS variables are processed */
__attribute__((noinline, used))
static void process_tls_variables(void) {
    /* 1. Weak TLS variable - should trigger DECL_WEAK copying */
    __thread int tls_weak __attribute__((weak));
    tls_weak = 0x1234;
    tls_addresses[0] = (uintptr_t)&tls_weak;
    
    /* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
    __thread int tls_hidden __attribute__((visibility("hidden")));
    tls_hidden = 0x5678;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    
    /* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
    __thread int tls_common;  /* No initializer = tentative definition */
    tls_common = 0x9ABC;
    tls_addresses[2] = (uintptr_t)&tls_common;
    
    /* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
    /* We'll define this below as a separate definition */
    extern __thread int tls_external;
    tls_addresses[3] = (uintptr_t)&tls_external;
    
    /* 5. Preserved TLS variable - may influence DECL_PRESERVE_P */
    /* Use 'used' attribute to ensure it's preserved */
    __thread int tls_preserved __attribute__((used));
    tls_preserved = 0xDEF0;
    tls_addresses[4] = (uintptr_t)&tls_preserved;
    
    /* 6. Public TLS variable with initializer - triggers TREE_PUBLIC */
    __thread int tls_public = 0x1111;
    tls_addresses[5] = (uintptr_t)&tls_public;
    
    /* 7. Static TLS variable (non-public context) - affects DECL_CONTEXT */
    static __thread int tls_static;
    tls_static = 0x2222;
    tls_addresses[6] = (uintptr_t)&tls_static;
    
    /* 8. TLS variable used in computation to prevent dead code elimination */
    __thread int tls_computed = 1;
    for (int i = 0; i < 10; i++) {
        tls_computed = tls_computed * 3 + 1;
    }
    tls_addresses[7] = (uintptr_t)&tls_computed + tls_computed;
    
    /* Force TREE_USED flag by actually using the variables */
    int sum = tls_weak + tls_hidden + tls_common + tls_external + 
              tls_preserved + tls_public + tls_static + tls_computed;
    tls_addresses[8] = sum;
}

/* Windows-specific DLL import simulation */
#ifdef _WIN32
/* Simulate DLL import attribute */
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)

DLL_IMPORT __thread int tls_dllimport;
#else
/* For non-Windows, use visibility attributes */
__thread int tls_dllimport __attribute__((visibility("default")));
#endif

/* Definition of the external TLS variable */
__thread int tls_external = 0x8888;

/* Another function that uses TLS to ensure multiple contexts */
__attribute__((noinline))
static void another_tls_user(void) {
    __thread int tls_local_func __attribute__((used));
    tls_local_func = 0x5555;
    tls_addresses[9] = (uintptr_t)&tls_local_func;
    
    /* Use the DLL import TLS variable if defined */
#ifdef _WIN32
    tls_dllimport = 0x7777;
#endif
}

/* Main function that orchestrates everything */
int main(void) {
    printf("Testing emulated TLS coverage...\n");
    
    /* Process TLS variables in helper function */
    process_tls_variables();
    
    /* Process in another function */
    another_tls_user();
    
    /* Force references to all TLS variables to prevent optimization */
    volatile int dummy = 0;
    
    /* Create checksum from TLS addresses to ensure they're used */
    uintptr_t checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum ^= tls_addresses[i];
        checksum = (checksum << 1) | (checksum >> (sizeof(checksum)*8 - 1));
    }
    
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    
    /* Force comparison of TLS addresses (prevents optimization) */
    __thread int tls_a = 1;
    __thread int tls_b = 2;
    
    if (&tls_a != &tls_b) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Use thread-local variables in main thread */
    tls_a = 100;
    tls_b = 200;
    printf("tls_a + tls_b = %d\n", tls_a + tls_b);
    
    /* Check if we're likely using emulated TLS */
#ifdef USE_EMULATED_TLS
    printf("Using emulated TLS (__HAVE_TLS not defined)\n");
#else
    printf("Native TLS might be available\n");
#endif
    
    return 0;
}
