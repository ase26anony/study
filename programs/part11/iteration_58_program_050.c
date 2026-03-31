/* test_emutls_coverage.c
 * This program tests GCC's emulated TLS initialization by creating
 * thread-local variables with diverse attributes to cover lines 295-304
 * in tree-emutls.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

/* Global volatile array to prevent optimization */
volatile void* tls_addresses[10];
volatile int tls_values[10];

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

/* 6. Public TLS variable with initializer - ensures TREE_PUBLIC is set */
__thread int tls_public = 42;

/* 7. TLS variable used in a way that prevents optimization */
__thread int tls_used = 100;

/* 8. Static TLS (non-public) for contrast */
static __thread int tls_static = 200;

/* 9. TLS with both weak and visibility attributes */
__thread int tls_weak_hidden __attribute__((weak, visibility("default")));

/* 10. TLS variable that will be DLL imported on Windows */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* On non-Windows, simulate with extern */
extern __thread int tls_dllimport;
#endif

/* ========== HELPER FUNCTIONS ========== */

/* Noinline function to ensure TLS variables are processed */
__attribute__((noinline, used))
static void process_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = &tls_weak;
    tls_addresses[1] = &tls_hidden;
    tls_addresses[2] = &tls_common;
    tls_addresses[3] = &tls_external;
    tls_addresses[4] = &tls_preserved;
    tls_addresses[5] = &tls_public;
    tls_addresses[6] = &tls_used;
    tls_addresses[7] = &tls_static;
    tls_addresses[8] = &tls_weak_hidden;
    tls_addresses[9] = &tls_dllimport;
    
    /* Use the values to prevent dead code elimination */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = tls_external;
    tls_values[4] = tls_preserved;
    tls_values[5] = tls_public;
    tls_values[6] = tls_used;
    tls_values[7] = tls_static;
    tls_values[8] = tls_weak_hidden;
    tls_values[9] = tls_dllimport;
    
    /* Modify some TLS variables */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_public++;
    tls_used += 5;
}

/* Another function that uses TLS in a different context */
__attribute__((noinline))
static int compute_tls_checksum(void) {
    int sum = 0;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_external;
    sum += tls_preserved;
    sum += tls_public;
    sum += tls_used;
    sum += tls_static;
    sum += tls_weak_hidden;
    sum += tls_dllimport;
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int i;
    
    /* Initialize some TLS variables */
    tls_common = 10;
    tls_preserved = 20;
    tls_weak_hidden = 30;
    
    /* Process TLS variables in helper function */
    process_tls_variables();
    
    /* Compute checksum to ensure all TLS variables are used */
    int checksum = compute_tls_checksum();
    
    /* Force referencing of variables for address comparison */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print some values to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    printf("tls_public value: %d\n", tls_public);
    printf("tls_used value: %d\n", tls_used);
    
    /* Verify addresses were captured */
    printf("Address of tls_hidden: %p\n", (void*)tls_addresses[1]);
    printf("Address of tls_common: %p\n", (void*)tls_addresses[2]);
    
    return 0;
}

/* ========== SECOND TRANSLATION UNIT SIMULATION ========== */
/* 
 * Normally this would be in a separate file, but for simplicity
 * we'll use weak symbols and conditional compilation
 */

/* Define the external TLS variable */
#ifdef DEFINE_EXTERNAL_TLS
__thread int tls_external = 99;

/* Define the DLL import variable for Windows */
#ifdef _WIN32
__declspec(dllexport) __thread int tls_dllimport = 77;
#else
__thread int tls_dllimport = 77;
#endif

/* Weak definition for weak TLS variables */
__thread int tls_weak = 55;
__thread int tls_weak_hidden = 66;
#endif
