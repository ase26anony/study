/* test-emutls-coverage.c
 * Comprehensive test to cover emulated TLS attribute copying in tree-emutls.cc
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 test-emutls-coverage.c -o test-emutls
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is not available */
#ifndef __HAVE_TLS
#define EMULATED_TLS 1
#else
/* Still try to force emulated mode with compilation flags */
#define EMULATED_TLS 1
#endif

/* Global volatile array to prevent optimization */
volatile uintptr_t tls_addresses[10];
volatile int tls_values[10];
int checksum = 0;

/* ===== TLS VARIABLES WITH VARIOUS ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. Public TLS variable with used attribute - influences TREE_PUBLIC and DECL_PRESERVE_P */
__thread int tls_public __attribute__((used));

/* 5. TLS variable with both used and noinline function dependency - for DECL_PRESERVE_P */
__thread int tls_preserved;

/* 6. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 7. External TLS declaration (will be defined separately) */
extern __thread int tls_external;

/* 8. TLS variable in function scope - affects DECL_CONTEXT */
static void function_with_tls(void) {
    static __thread int tls_in_function = 100;
    tls_addresses[7] = (uintptr_t)&tls_in_function;
    tls_values[7] = tls_in_function++;
}

/* 9. TLS with section attribute - additional attribute testing */
__thread int tls_sectioned __attribute__((section(".tls_data")));

/* Conditional Windows DLL import attribute */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__declspec(dllimport) __thread int tls_imported;
#else
/* For non-Windows, use a regular TLS variable */
__thread int tls_imported;
#endif

/* ===== NOINLINE HELPER FUNCTIONS ===== */

/* Force preservation of TLS variables by using them in noinline functions */
__attribute__((noinline, used))
static void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = (uintptr_t)&tls_weak;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    tls_addresses[2] = (uintptr_t)&tls_common;
    tls_addresses[3] = (uintptr_t)&tls_public;
    tls_addresses[4] = (uintptr_t)&tls_preserved;
    tls_addresses[5] = (uintptr_t)&tls_init;
    tls_addresses[6] = (uintptr_t)&tls_external;
    tls_addresses[8] = (uintptr_t)&tls_sectioned;
    tls_addresses[9] = (uintptr_t)&tls_imported;
    
    /* Use the TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_public = 4;
    tls_preserved = 5;
    /* tls_init already initialized to 42 */
    /* tls_external will be set externally */
    tls_sectioned = 8;
    tls_imported = 9;
    
    /* Compute checksum using all TLS variables */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = tls_public;
    tls_values[4] = tls_preserved;
    tls_values[5] = tls_init;
    tls_values[6] = 0; /* Placeholder for external */
    tls_values[8] = tls_sectioned;
    tls_values[9] = tls_imported;
}

__attribute__((noinline, used))
static void compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += tls_values[i];
    }
    checksum = sum;
    
    /* Force comparison of TLS addresses */
    if (&tls_weak != &tls_hidden) {
        checksum += 1000; /* Different addresses - good! */
    }
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    /* Initialize some TLS variables */
    tls_common = 10;
    tls_public = 20;
    tls_preserved = 30;
    
    /* Call function with function-scope TLS */
    function_with_tls();
    
    /* Use TLS variables in noinline function to ensure processing */
    use_tls_variables();
    
    /* Set external TLS variable if available */
    tls_external = 7;
    tls_values[6] = tls_external;
    
    /* Compute final checksum */
    compute_checksum();
    
    /* Print results to ensure no dead code elimination */
    printf("TLS Coverage Test Results:\n");
    printf("  Checksum: %d\n", checksum);
    printf("  tls_weak address: %p\n", (void*)tls_addresses[0]);
    printf("  tls_hidden address: %p\n", (void*)tls_addresses[1]);
    printf("  tls_common value: %d\n", tls_common);
    printf("  tls_init value: %d\n", tls_init);
    
    /* Verify TLS addresses are different */
    if (tls_addresses[0] != tls_addresses[1]) {
        printf("  TLS addresses differ - emulation likely active\n");
    }
    
#ifdef EMULATED_TLS
    printf("  Compiling with emulated TLS mode\n");
#endif
    
    return 0;
}

/* ===== SECONDARY FILE FOR EXTERNAL TLS ===== */
/* 
 * To fully test DECL_EXTERNAL, compile this separately:
 * 
 * // external_tls.c
 * __thread int tls_external = 77;
 * 
 * Then link with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 test-emutls-coverage.c external_tls.c -o test-emutls
 */
