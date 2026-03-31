/* tls_emutest.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && __GNUC__ >= 4
#pragma GCC tls_model emulated
#endif

#include <stdio.h>
#include <stdint.h>

/* Prevent optimizations */
#define NOINLINE __attribute__((noinline, noclone))
#define USED __attribute__((used))

/* Global volatile array to prevent optimization */
volatile void* tls_addresses[10];
volatile int checksum = 0;

/* ============================================
   TLS VARIABLES WITH DIFFERENT ATTRIBUTES
   ============================================ */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 5. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 6. Public/External TLS declaration */
/* First declare as external */
extern __thread int tls_external;

/* 7. TLS variable in function scope for DECL_CONTEXT variation */
static void func_with_tls(void) {
    static __thread int tls_local_func = 100;
    tls_addresses[6] = (void*)&tls_local_func;
}

/* 8. TLS with multiple attributes combined */
__thread int tls_combined __attribute__((weak, visibility("default")));

/* Windows-specific DLL import simulation */
#ifdef _WIN32
/* 9. DLL Import TLS - triggers DECL_DLLIMPORT_P */
__declspec(dllimport) __thread int tls_imported;
#else
/* For non-Windows, use visibility protected as alternative */
__thread int tls_imported __attribute__((visibility("protected")));
#endif

/* ============================================
   HELPER FUNCTIONS TO USE TLS VARIABLES
   ============================================ */

NOINLINE static void use_tls_variables(void) {
    /* Take addresses of all TLS variables to force their instantiation */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_init;
    tls_addresses[4] = (void*)&tls_preserved;
    tls_addresses[5] = (void*)&tls_external;
    tls_addresses[7] = (void*)&tls_combined;
    tls_addresses[8] = (void*)&tls_imported;
    
    /* Modify and use TLS variables to prevent dead code elimination */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = tls_init + 1;
    
    /* Create dependency chain */
    int local_sum = tls_weak + tls_hidden + tls_common + tls_init;
    
    /* Use the external TLS variable */
    if (&tls_external != NULL) {
        local_sum += 10;
    }
    
    /* Store checksum in global volatile to prevent optimization */
    checksum = local_sum;
    
    /* Call function with local TLS */
    func_with_tls();
}

NOINLINE static void verify_tls_addresses(void) {
    /* Force comparison of TLS addresses */
    if (&tls_weak == &tls_hidden) {
        checksum += 1000; /* Unlikely branch */
    }
    
    /* Ensure all addresses are different (they should be) */
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 9; j++) {
            if (tls_addresses[i] == tls_addresses[j] && tls_addresses[i] != NULL) {
                checksum += 1;
            }
        }
    }
}

/* ============================================
   MAIN FUNCTION
   ============================================ */

int main(void) {
    printf("Testing emulated TLS with various attributes...\n");
    
    /* Initialize some TLS variables */
    tls_common = 5;
    tls_preserved = 7;
    tls_combined = 9;
    
    /* Use TLS variables in helper function */
    use_tls_variables();
    
    /* Verify addresses */
    verify_tls_addresses();
    
    /* Print results to ensure variables are used */
    printf("tls_weak address: %p\n", (void*)&tls_weak);
    printf("tls_hidden address: %p\n", (void*)&tls_hidden);
    printf("tls_common value: %d\n", tls_common);
    printf("tls_init value: %d\n", tls_init);
    printf("tls_preserved value: %d\n", tls_preserved);
    printf("Checksum: %d\n", checksum);
    
    /* Runtime check for TLS emulation */
#ifdef __HAVE_TLS
    printf("Native TLS support detected\n");
#else
    printf("Using emulated TLS\n");
#endif
    
    return checksum != 0 ? 0 : 1;
}

/* ============================================
   SECOND TRANSLATION UNIT (if split compilation)
   ============================================ */
/* 
   For full testing with DECL_EXTERNAL, compile this separately:
   
   File: tls_external.c
   
   __thread int tls_external = 123;
   
   Then link with: gcc -O2 -ftls-model=emulated tls_emutest.c tls_external.c -o tls_test
*/

/* Inline definition for single-file compilation */
__thread int tls_external = 123;
