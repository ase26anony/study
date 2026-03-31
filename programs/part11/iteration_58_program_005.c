/* test_emutls_coverage.c
 * 
 * This program creates thread-local variables with diverse attributes
 * to trigger the property copying logic in GCC's emulated TLS implementation.
 * Specifically targets lines 295-304 in tree-emutls.cc.
 */

/* Force emulated TLS mode if supported by compiler */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of TLS variable usage */
volatile void *volatile tls_addresses[10];
volatile int volatile tls_values[10];
int global_counter = 0;

/* Helper function to ensure TLS variables are used in non-optimizable ways */
__attribute__((noinline, used))
static void use_tls_variables(void) {
    /* Take addresses and store in volatile array to prevent optimization */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_external;
    tls_addresses[4] = (void*)&tls_init;
    
    /* Use values to ensure they're not dead code */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = tls_external;
    tls_values[4] = tls_init;
    
    /* Force computation with TLS variables */
    global_counter += tls_weak + tls_hidden + tls_common + tls_external + tls_init;
}

/* ===== TLS VARIABLES WITH VARIOUS ATTRIBUTES ===== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable - tentative definition at file scope, may set DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - will be defined in another TU or with extern */
/* First declare as external */
extern __thread int tls_external;

/* 5. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 6. TLS variable with preserve attribute - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. Public TLS variable - TREE_PUBLIC */
__thread int tls_public;

/* 8. TLS variable in function scope - affects DECL_CONTEXT */
static void function_with_tls(void) {
    static __thread int tls_in_function = 100;
    tls_addresses[5] = (void*)&tls_in_function;
    tls_values[5] = tls_in_function++;
}

/* 9. DLL Import simulation (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__MINGW32__)
__attribute__((dllimport)) __thread int tls_dllimport;
#endif

/* ===== SECONDARY DEFINITIONS ===== */

/* Define the external TLS variable (simulating separate compilation unit) */
/* In a real multi-TU test, this would be in a separate file */
__thread int tls_external = 123;

/* ===== MAIN FUNCTION ===== */

int main(void) {
    int i;
    
    printf("Testing emulated TLS with various attributes...\n");
    
    /* Initialize some TLS variables */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_public = 4;
    tls_preserved = 5;
    
    /* Use the TLS variables in non-optimizable ways */
    use_tls_variables();
    function_with_tls();
    
    /* Force referencing of all TLS variables */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Compute checksum using TLS variables */
    uintptr_t checksum = 0;
    for (i = 0; i < 6; i++) {
        checksum += (uintptr_t)tls_addresses[i];
        checksum += tls_values[i];
    }
    
    /* Print something dependent on TLS values to prevent optimization */
    printf("TLS checksum: %lu\n", (unsigned long)checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Print individual TLS values */
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_common: %d\n", tls_common);
    printf("tls_external: %d\n", tls_external);
    printf("tls_init: %d\n", tls_init);
    printf("tls_public: %d\n", tls_public);
    printf("tls_preserved: %d\n", tls_preserved);
    
    /* Check if we're likely using emulated TLS */
#ifdef __HAVE_TLS
    printf("Native TLS support detected\n");
#else
    printf("Likely using emulated TLS\n");
#endif
    
    return 0;
}
