/* test_emutls_coverage.c
 * Comprehensive test for GCC emulated TLS coverage
 * Targets lines 295-304 in tree-emutls.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode if supported by compiler */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC tls_model emulated
#endif

/* Prevent optimization of TLS variable usage */
volatile void* volatile_global_ptr;
volatile int volatile_global_int;

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable - tentative definition, may set DECL_COMMON */
__thread int tls_common;

/* 4. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 5. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 6. Public/External declaration simulation */
/* First declare as extern */
extern __thread int tls_external;

/* Then define it (simulating separate compilation unit) */
__thread int tls_external = 100;

/* 7. TLS variable with noinline function context */
static __attribute__((noinline)) void use_tls_in_function(void) {
    static __thread int tls_in_function = 99;
    volatile_global_int = tls_in_function++;
}

/* 8. TLS with section attribute */
__thread int tls_sectioned __attribute__((section(".tls_data"))) = 77;

/* Windows-specific DLL import simulation */
#ifdef _WIN32
/* For MinGW/Windows targets */
__declspec(dllimport) __thread int tls_imported;
#else
/* For non-Windows, simulate with visibility */
__thread int tls_imported __attribute__((visibility("default")));
#endif

/* ===== HELPER FUNCTIONS TO FORCE TLS USAGE ===== */

/* Force compiler to process TLS variables by taking addresses */
__attribute__((noinline, used))
static void capture_tls_addresses(uintptr_t* addresses) {
    addresses[0] = (uintptr_t)&tls_weak;
    addresses[1] = (uintptr_t)&tls_hidden;
    addresses[2] = (uintptr_t)&tls_common;
    addresses[3] = (uintptr_t)&tls_init;
    addresses[4] = (uintptr_t)&tls_preserved;
    addresses[5] = (uintptr_t)&tls_external;
    addresses[6] = (uintptr_t)&tls_sectioned;
    addresses[7] = (uintptr_t)&tls_imported;
    
    /* Use the variables to prevent dead code elimination */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = 4;
    tls_imported = 5;
    
    /* Force computation with TLS values */
    volatile_global_int = tls_weak + tls_hidden + tls_common + 
                         tls_init + tls_preserved + tls_external +
                         tls_sectioned + tls_imported;
}

/* Another function to create different DECL_CONTEXT */
__attribute__((noinline))
static void nested_tls_usage(void) {
    /* TLS in nested scope */
    static __thread int tls_nested = 123;
    tls_nested++;
    volatile_global_int = tls_nested;
    
    /* Use the function-scoped TLS */
    use_tls_in_function();
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    uintptr_t tls_addresses[8] = {0};
    
    printf("Testing emulated TLS coverage...\n");
    
    /* Capture all TLS addresses to force their instantiation */
    capture_tls_addresses(tls_addresses);
    
    /* Use nested context */
    nested_tls_usage();
    
    /* Force comparison of TLS addresses (prevents optimization) */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Compute checksum from TLS addresses */
    uintptr_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum ^= tls_addresses[i];
    }
    
    /* Use checksum to prevent optimization */
    volatile_global_ptr = (void*)checksum;
    
    /* Print some values to ensure TLS is used */
    printf("tls_init = %d\n", tls_init);
    printf("tls_external = %d\n", tls_external);
    printf("tls_sectioned = %d\n", tls_sectioned);
    
    /* Modify and use TLS variables */
    tls_common = 999;
    tls_preserved = 888;
    
    printf("Modified values: tls_common=%d, tls_preserved=%d\n", 
           tls_common, tls_preserved);
    
    /* Final check to force all TLS variables to be considered */
    if (tls_addresses[0] != 0 && checksum != 0) {
        printf("TLS test completed successfully\n");
    }
    
    return 0;
}

/* ===== SECOND FILE SIMULATION (for external linkage) ===== */
/* Normally this would be in a separate compilation unit */
/* Uncomment and compile separately if needed */

/*
// File2.c for multi-translation unit test
__thread int external_tls_definition = 0xABCD;

// Function that uses TLS from main file (if linked together)
void use_external_tls(void) {
    extern __thread int tls_external;
    tls_external = external_tls_definition;
}
*/
