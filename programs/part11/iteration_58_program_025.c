/* test-emutls-attributes.c
 * 
 * This test program creates thread-local variables with diverse attributes
 * to trigger the property copying logic in GCC's emutls_decl function.
 * The goal is to cover lines 295-304 in tree-emutls.cc.
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent dead code elimination */
volatile void *volatile tls_addresses[10];
volatile int volatile tls_values[10];
int checksum = 0;

/* Force emulated TLS by targeting older architecture */
#ifdef __x86_64__
#undef __x86_64__
#endif

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable - tentative definition, may set DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - will be defined in another TU or later */
extern __thread int tls_external;

/* 5. Public/used TLS variable - ensures TREE_PUBLIC and TREE_USED */
__thread int tls_public __attribute__((used));

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. Initialized TLS variable - ensures it's not just BSS */
__thread int tls_initialized = 42;

/* 8. TLS with section attribute - additional complexity */
__thread int tls_in_section __attribute__((section(".tdata"))) = 100;

/* 9. Static TLS inside function - different DECL_CONTEXT */
static void function_with_static_tls(void) {
    static __thread int tls_static_inside_func = 99;
    tls_addresses[8] = (void*)&tls_static_inside_func;
    tls_values[8] = tls_static_inside_func;
}

/* 10. TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64)));

/* Conditional Windows DLL import attributes */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__declspec(dllimport) __thread int tls_imported;
#else
/* For non-Windows, we'll simulate with a regular extern */
extern __thread int tls_imported;
#endif

/* ========== HELPER FUNCTIONS ========== */

/* __attribute__((noinline)) ensures function isn't optimized away */
__attribute__((noinline)) 
static void use_all_tls_variables(void) {
    /* Take addresses to prevent optimization */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_external;
    tls_addresses[4] = (void*)&tls_public;
    tls_addresses[5] = (void*)&tls_preserved;
    tls_addresses[6] = (void*)&tls_initialized;
    tls_addresses[7] = (void*)&tls_in_section;
    tls_addresses[9] = (void*)&tls_aligned;
    
    /* Use values to ensure they're accessed */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_public = 4;
    tls_preserved = 5;
    tls_aligned = 6;
    
    /* Compute a checksum using all TLS variables */
    int local_sum = 0;
    local_sum += tls_weak;
    local_sum += tls_hidden;
    local_sum += tls_common;
    local_sum += tls_public;
    local_sum += tls_preserved;
    local_sum += tls_initialized;
    local_sum += tls_in_section;
    local_sum += tls_aligned;
    
    /* Store in volatile to prevent optimization */
    tls_values[0] = local_sum;
    
    /* Force different addresses check */
    if (&tls_weak != &tls_hidden) {
        tls_values[1] = 1;
    }
}

/* Another noinline function to ensure TLS variables are processed */
__attribute__((noinline, used))
static void modify_tls_variables(void) {
    tls_weak += 1;
    tls_hidden *= 2;
    tls_common = tls_weak + tls_hidden;
    tls_public ^= 0x55;
    tls_preserved = ~tls_preserved;
    tls_initialized = tls_initialized * 3 + 7;
    tls_in_section = tls_in_section / 2;
    tls_aligned = tls_aligned | 0xF0;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Define the external TLS variable if not defined elsewhere */
    __thread int tls_external = 123;
    
    /* Define the imported TLS variable for non-Windows */
#ifndef _WIN32
#ifndef __MINGW32__
    __thread int tls_imported = 456;
#endif
#endif
    
    /* Call helper functions that use TLS variables */
    use_all_tls_variables();
    modify_tls_variables();
    
    /* Call function with static TLS */
    function_with_static_tls();
    
    /* Use the TLS variables in main to ensure they're referenced */
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_common;
    checksum += tls_external;
    checksum += tls_public;
    checksum += tls_preserved;
    checksum += tls_initialized;
    checksum += tls_in_section;
    checksum += tls_aligned;
    
    /* Print something to prevent entire program optimization */
    printf("TLS test checksum: %d\n", checksum);
    printf("TLS addresses differ: %s\n", 
           (&tls_weak != &tls_hidden) ? "yes" : "no");
    
    /* Print addresses (volatile stored) to ensure they're used */
    for (int i = 0; i < 10; i++) {
        if (tls_addresses[i] != 0) {
            printf("TLS address %d: %p\n", i, (void*)tls_addresses[i]);
        }
    }
    
    return checksum != 0 ? 0 : 1;
}

/* ========== SECOND TRANSLATION UNIT SIMULATION ========== */
/* 
 * Normally this would be in a separate file, but for simplicity
 * we'll use a conditional compilation section.
 * To truly test external/DLL import, compile this as two separate files.
 */

#ifdef COMPILE_SECOND_FILE
/* File2.c - defines external TLS variables */
__thread int tls_external = 789;

#ifdef _WIN32
__declspec(dllexport) __thread int tls_imported = 999;
#elif defined(__MINGW32__)
__declspec(dllexport) __thread int tls_imported = 999;
#endif
#endif
