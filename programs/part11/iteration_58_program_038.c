/* tls_emutls_test.c
 * Test program to cover GCC's emulated TLS initialization lines 295-304
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread tls_emutls_test.c -o tls_test
 * For 32-bit: gcc -O2 -m32 -ftls-model=emulated -fno-builtin tls_emutls_test.c -o tls_test32
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is available */
#ifndef __HAVE_TLS
#define USE_EMULATED_TLS 1
#endif

/* Prevent optimization of TLS variable usage */
volatile void* volatile tls_addresses[10];
volatile int volatile tls_values[10];
int volatile checksum = 0;

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may set DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - will be defined in another TU or later */
extern __thread int tls_external;

/* 5. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 6. Public TLS variable with initializer - affects TREE_PUBLIC */
__thread int tls_public = 42;

/* 7. Static TLS variable (non-public context) - affects DECL_CONTEXT */
static __thread int tls_static = 100;

/* 8. Another weak TLS with initializer */
__thread int tls_weak_init __attribute__((weak)) = 200;

/* 9. TLS with both weak and visibility attributes */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden")));

/* 10. TLS in a different linkage context */
__thread int tls_internal __attribute__((visibility("internal")));

/* For Windows DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar concept on non-Windows */
extern __thread int tls_imported;
#endif

/* ========== HELPER FUNCTIONS ========== */

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline, used)) 
static void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_external;
    tls_addresses[4] = (void*)&tls_preserved;
    tls_addresses[5] = (void*)&tls_public;
    tls_addresses[6] = (void*)&tls_static;
    tls_addresses[7] = (void*)&tls_weak_init;
    tls_addresses[8] = (void*)&tls_weak_hidden;
    tls_addresses[9] = (void*)&tls_internal;
    
    /* Use TLS variable values in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = 4;
    tls_static = 5;
    tls_weak_init = 6;
    tls_weak_hidden = 7;
    tls_internal = 8;
    
    /* Force computation with TLS values */
    int sum = tls_weak + tls_hidden + tls_common + tls_preserved +
              tls_public + tls_static + tls_weak_init + tls_weak_hidden +
              tls_internal;
    
    tls_values[0] = sum;
    checksum = sum;
}

/* Another function to create different DECL_CONTEXT usage */
__attribute__((noinline))
static void nested_tls_usage(void) {
    static __thread int nested_tls = 123;
    tls_values[1] = nested_tls++;
    
    /* Use the TLS variables in a different context */
    if (&tls_weak != &tls_hidden) {
        tls_values[2] = 1;
    }
}

/* Function to test external TLS */
__attribute__((noinline))
void test_external_tls(void) {
    /* This should trigger DECL_EXTERNAL handling */
    tls_external = 999;
    tls_values[3] = tls_external;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize some TLS variables */
    tls_common = 10;
    tls_public = 20;
    
    /* Use TLS variables in helper functions */
    use_tls_variables();
    nested_tls_usage();
    test_external_tls();
    
    /* Force references to all TLS variables to prevent optimization */
    volatile int* ptrs[] = {
        &tls_weak,
        &tls_hidden,
        &tls_common,
        &tls_external,
        &tls_preserved,
        &tls_public,
        &tls_static,
        &tls_weak_init,
        &tls_weak_hidden,
        &tls_internal
    };
    
    /* Compute a runtime checksum using TLS addresses and values */
    uintptr_t addr_sum = 0;
    for (int i = 0; i < 10; i++) {
        if (ptrs[i]) {
            addr_sum += (uintptr_t)ptrs[i];
        }
    }
    
    /* Print results to ensure all code is executed */
    printf("TLS test results:\n");
    printf("  Checksum from TLS values: %d\n", checksum);
    printf("  Address sum: %lu\n", (unsigned long)addr_sum);
    printf("  tls_public value: %d\n", tls_public);
    printf("  tls_common value: %d\n", tls_common);
    
    /* Force comparison of TLS addresses (triggers more TLS processing) */
    if (&tls_weak != &tls_hidden) {
        printf("  TLS addresses differ as expected\n");
    }
    
    /* Check if we're using emulated TLS */
#ifdef USE_EMULATED_TLS
    printf("  Using emulated TLS\n");
#else
    printf("  Native TLS available\n");
#endif
    
    return 0;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This could be in a separate file, but including here for simplicity */
__thread int tls_external = 555;

/* For Windows simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int tls_imported = 777;
#else
__thread int tls_imported = 777;
#endif
