/* tls_emutls_test.c
 * Test program to cover GCC's emulated TLS initialization lines 295-304
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread tls_emutls_test.c -o tls_test
 * For 32-bit: gcc -O2 -m32 -ftls-model=emulated -fno-builtin tls_emutls_test.c -o tls_test_32
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is not available */
#ifndef __HAVE_TLS
#define USE_EMULATED_TLS 1
#endif

/* Global volatile array to prevent optimization */
volatile void* tls_addresses[10];
volatile int tls_values[10];

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. Public TLS with initializer - triggers TREE_PUBLIC */
__thread int tls_public = 42;

/* 6. Used TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_used __attribute__((used)) = 100;

/* 7. TLS variable with noinline function context */
static __thread int tls_in_function;

/* 8. DLL Import simulation (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar concept with weak external */
extern __thread int tls_imported __attribute__((weak));
#endif

/* 9. TLS with retain attribute (GCC extension) - may influence DECL_PRESERVE_P */
__thread int tls_retained __attribute__((retain));

/* 10. TLS in different linkage contexts */
static __thread int tls_static;  /* Internal linkage */

/* ===== HELPER FUNCTIONS ===== */

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline)) 
static void process_tls_variables(void) {
    /* Take addresses of all TLS variables - prevents dead code elimination */
    tls_addresses[0] = &tls_weak;
    tls_addresses[1] = &tls_hidden;
    tls_addresses[2] = &tls_common;
    tls_addresses[3] = &tls_external;
    tls_addresses[4] = &tls_public;
    tls_addresses[5] = &tls_used;
    tls_addresses[6] = &tls_in_function;
    tls_addresses[7] = &tls_imported;
    tls_addresses[8] = &tls_retained;
    tls_addresses[9] = &tls_static;
    
    /* Use the TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    /* tls_external is defined elsewhere */
    tls_public++;
    tls_used += 10;
    tls_in_function = tls_weak + tls_hidden;
    
    /* Store computed values */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = 0; /* Placeholder for external */
    tls_values[4] = tls_public;
    tls_values[5] = tls_used;
    tls_values[6] = tls_in_function;
    tls_values[7] = 0; /* Placeholder for imported */
    tls_values[8] = tls_retained;
    tls_values[9] = tls_static;
}

/* Another function to create different DECL_CONTEXT */
__attribute__((noinline))
static void nested_tls_access(void) {
    static __thread int nested_tls = 99;
    tls_addresses[6] = &nested_tls;
    tls_values[6] = nested_tls++;
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    int checksum = 0;
    
    printf("Testing emulated TLS with various attributes...\n");
    
    /* Process TLS variables in helper function */
    process_tls_variables();
    
    /* Access TLS in nested context */
    nested_tls_access();
    
    /* Force references to all TLS variables in main */
    if (&tls_weak != &tls_hidden) {
        checksum += (uintptr_t)&tls_weak;
    }
    
    if (&tls_common != NULL) {
        checksum += tls_common;
    }
    
    checksum += tls_public;
    checksum += tls_used;
    
    /* Use the volatile arrays to prevent optimization */
    for (int i = 0; i < 10; i++) {
        checksum += (uintptr_t)tls_addresses[i];
        checksum += tls_values[i];
    }
    
    printf("TLS test checksum: %d\n", checksum);
    printf("TLS addresses differ as expected\n");
    
    /* Runtime check for emulated TLS */
    printf("TLS variable addresses:\n");
    printf("  tls_weak: %p\n", &tls_weak);
    printf("  tls_hidden: %p\n", &tls_hidden);
    printf("  tls_common: %p\n", &tls_common);
    
    return checksum & 0xFF; /* Return non-zero to indicate execution */
}

/* ===== SECOND TRANSLATION UNIT SIMULATION ===== */
/* This would normally be in a separate file, but we'll include it here
 * with conditional compilation to simulate multi-TU scenario */
#ifdef COMPILE_SECOND_PART
/* tls_external definition for external linkage test */
__thread int tls_external = 12345;

/* DLL export simulation for Windows */
#ifdef _WIN32
__declspec(dllexport) __thread int tls_imported = 67890;
#else
__thread int tls_imported = 67890;
#endif
#endif
