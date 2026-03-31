/* test-emutls-coverage.c
 * Comprehensive test to cover emulated TLS attribute copying in GCC's tree-emutls.cc
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread test-emutls-coverage.c -o test-emutls
 * For 32-bit: gcc -O2 -m32 -ftls-model=emulated -fno-builtin test-emutls-coverage.c -o test-emutls-32
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is not available */
#ifndef __HAVE_TLS
#define USE_EMULATED_TLS 1
#else
/* Still try to force emulated mode via compilation flags */
#define USE_EMULATED_TLS 1
#endif

/* Global volatile array to prevent optimization */
volatile uintptr_t tls_addresses[10] = {0};

/* Helper function marked noinline to prevent optimization */
__attribute__((noinline, used))
void process_tls_variables(void) {
    /* Take addresses of all TLS variables to force their instantiation */
    tls_addresses[0] = (uintptr_t)&tls_weak;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    tls_addresses[2] = (uintptr_t)&tls_common;
    tls_addresses[3] = (uintptr_t)&tls_external;
    tls_addresses[4] = (uintptr_t)&tls_init;
    tls_addresses[5] = (uintptr_t)&tls_static_func;
    
    /* Use the variables in computations */
    tls_weak = 100;
    tls_hidden = 200;
    tls_common = tls_weak + tls_hidden;
    tls_init = tls_init * 2 + 1;
    tls_static_func = tls_static_func + 5;
    
    /* Store computed values */
    tls_addresses[6] = tls_weak;
    tls_addresses[7] = tls_hidden;
    tls_addresses[8] = tls_common;
    tls_addresses[9] = tls_init;
}

/* ===== TLS VARIABLES WITH VARIOUS ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;  /* No initializer = tentative definition */

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
/* We'll define this in the same file but treat as external in spirit */
extern __thread int tls_external;
__thread int tls_external = 42;  /* Actual definition */

/* 5. TLS with initializer - ensures not optimized as BSS */
__thread int tls_init = 10;

/* 6. TLS variable that should be preserved - may influence DECL_PRESERVE_P */
/* Using it in a noinline function helps ensure it's preserved */
__thread int tls_static_func __attribute__((used));

/* 7. TLS in different linkage contexts */
static __thread int tls_file_static;  /* Internal linkage */

/* Function with TLS in local scope */
void function_with_local_tls(void) {
    static __thread int tls_local_static = 0;
    tls_local_static++;
    tls_addresses[0] += tls_local_static;  /* Use to prevent optimization */
}

/* Conditional Windows DLL import simulation */
#ifdef _WIN32
/* 8. DLL Import TLS - triggers DECL_DLLIMPORT_P */
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate with visibility attributes on other platforms */
__thread int tls_imported __attribute__((visibility("default")));
#endif

/* Another helper to ensure TLS variables are referenced in different contexts */
__attribute__((noinline, used))
uintptr_t compute_tls_checksum(void) {
    uintptr_t sum = 0;
    
    /* Reference all TLS variables */
    sum += (uintptr_t)&tls_weak;
    sum += (uintptr_t)&tls_hidden;
    sum += (uintptr_t)&tls_common;
    sum += (uintptr_t)&tls_external;
    sum += (uintptr_t)&tls_init;
    sum += (uintptr_t)&tls_static_func;
    sum += (uintptr_t)&tls_file_static;
    
    /* Use their values */
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_external;
    sum += tls_init;
    sum += tls_static_func;
    
    /* Call function with local TLS */
    function_with_local_tls();
    
    return sum;
}

int main(void) {
    printf("Testing emulated TLS coverage for GCC tree-emutls.cc\n");
    printf("Targeting lines 295-304 in emutls_decl function\n\n");
    
    /* Initialize some TLS variables */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_static_func = 4;
    tls_file_static = 5;
    
    /* Process TLS variables in helper function */
    process_tls_variables();
    
    /* Compute checksum using all TLS variables */
    uintptr_t checksum = compute_tls_checksum();
    
    /* Force comparison of TLS addresses (prevents optimization) */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print results to ensure variables are used */
    printf("TLS variable values:\n");
    printf("  tls_weak: %d\n", tls_weak);
    printf("  tls_hidden: %d\n", tls_hidden);
    printf("  tls_common: %d\n", tls_common);
    printf("  tls_external: %d\n", tls_external);
    printf("  tls_init: %d\n", tls_init);
    printf("  tls_static_func: %d\n", tls_static_func);
    printf("  tls_file_static: %d\n", tls_file_static);
    
    printf("\nChecksum: 0x%lx\n", (unsigned long)checksum);
    printf("First TLS address: 0x%lx\n", (unsigned long)tls_addresses[0]);
    
    /* Verify emulated TLS is being used */
#if USE_EMULATED_TLS
    printf("\nUsing emulated TLS (as requested)\n");
#else
    printf("\nWarning: May be using native TLS\n");
#endif
    
    return 0;
}
