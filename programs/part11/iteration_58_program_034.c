/* test-emutls-coverage.c
 * Comprehensive test for GCC emulated TLS coverage (lines 295-304 in tree-emutls.cc)
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 -fPIC test-emutls-coverage.c -o test-emutls
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking and printing TLS mode */
#ifndef __HAVE_TLS
#define EMULATED_TLS 1
#else
#define EMULATED_TLS 0
#endif

/* Prevent optimization of TLS variable usage */
volatile void *tls_addresses[10];
volatile int tls_values[10];
int checksum = 0;

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;

/* 4. Public TLS variable with used attribute - affects TREE_PUBLIC and DECL_PRESERVE_P */
__thread int tls_public __attribute__((used));

/* 5. TLS variable with preserve attribute - triggers DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 6. External TLS declaration (defined below) - triggers DECL_EXTERNAL */
extern __thread int tls_external;

/* 7. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 8. TLS variable in different linkage context */
static __thread int tls_static = 100;

/* 9. TLS variable with section attribute */
__thread int tls_sectioned __attribute__((section(".tls_data")));

/* 10. TLS variable accessed via pointer */
__thread int *tls_pointer;

/* ========== WINDOWS-SPECIFIC ATTRIBUTES ========== */
#ifdef _WIN32
/* DLL Import TLS - triggers DECL_DLLIMPORT_P */
__declspec(dllimport) __thread int tls_imported;
#endif

/* ========== HELPER FUNCTIONS ========== */

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline, used))
void process_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_public;
    tls_addresses[4] = (void*)&tls_preserved;
    tls_addresses[5] = (void*)&tls_external;
    tls_addresses[6] = (void*)&tls_init;
    tls_addresses[7] = (void*)&tls_static;
    tls_addresses[8] = (void*)&tls_sectioned;
    tls_addresses[9] = (void*)&tls_pointer;
    
    /* Use TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_public = 4;
    tls_preserved = 5;
    tls_external = 6;
    tls_init = 7;
    tls_static = 8;
    tls_sectioned = 9;
    
    /* Initialize TLS pointer */
    tls_pointer = &tls_init;
    
    /* Compute checksum using all TLS variables */
    int sum = 0;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_public;
    sum += tls_preserved;
    sum += tls_external;
    sum += tls_init;
    sum += tls_static;
    sum += tls_sectioned;
    sum += (*tls_pointer);
    
    tls_values[0] = sum;
    checksum = sum;
}

/* Another function to create different DECL_CONTEXT */
__attribute__((noinline))
static void nested_tls_usage(void) {
    /* Local static TLS-like usage */
    static __thread int local_tls = 123;
    local_tls++;
    tls_values[1] = local_tls;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This would normally be in a separate file, but we define it here
 * with extern linkage to trigger DECL_EXTERNAL handling */
__thread int tls_external = 99;

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Print TLS mode information */
    printf("Emulated TLS mode: %s\n", EMULATED_TLS ? "YES" : "NO");
    
    /* Force TLS variable processing */
    process_tls_variables();
    nested_tls_usage();
    
    /* Force referencing of variables to prevent optimization */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Use all TLS variables in main to ensure they're referenced */
    volatile int *volatile ptr;
    ptr = &tls_weak; *ptr += 1;
    ptr = &tls_hidden; *ptr += 1;
    ptr = &tls_common; *ptr += 1;
    ptr = &tls_public; *ptr += 1;
    ptr = &tls_preserved; *ptr += 1;
    ptr = &tls_external; *ptr += 1;
    ptr = &tls_init; *ptr += 1;
    ptr = &tls_static; *ptr += 1;
    ptr = &tls_sectioned; *ptr += 1;
    
    /* Final checksum computation */
    int final_sum = 0;
    final_sum += tls_weak;
    final_sum += tls_hidden;
    final_sum += tls_common;
    final_sum += tls_public;
    final_sum += tls_preserved;
    final_sum += tls_external;
    final_sum += tls_init;
    final_sum += tls_static;
    final_sum += tls_sectioned;
    
    printf("Final checksum: %d\n", final_sum);
    printf("Global checksum: %d\n", checksum);
    
    /* Print TLS addresses to ensure they're unique */
    for (int i = 0; i < 10; i++) {
        printf("TLS address %d: %p\n", i, (void*)tls_addresses[i]);
    }
    
    return final_sum != 0 ? 0 : 1;
}
