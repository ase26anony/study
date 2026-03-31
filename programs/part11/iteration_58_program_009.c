/* test_emutls_coverage.c
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 test_emutls_coverage.c -o test_emutls
 * Or for Windows: gcc -O2 -ftls-model=emulated -D_WIN32 test_emutls_coverage.c -o test_emutls.exe
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is not available */
#ifndef __HAVE_TLS
#define USE_EMULATED_TLS 1
#endif

/* Prevent optimization of TLS variable usage */
volatile void *tls_addresses[10];
volatile int tls_values[10];
int checksum_result = 0;

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;  /* No initializer */

/* 4. External TLS declaration - triggers TREE_PUBLIC and DECL_EXTERNAL */
extern __thread int tls_external;
__thread int tls_external __attribute__((used)) = 4;

/* 5. DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, use a regular TLS variable */
__thread int tls_imported = 5;
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used)) = 6;

/* 7. TLS with no attributes (baseline) */
__thread int tls_plain = 7;

/* 8. Static TLS (non-public) to test different DECL_CONTEXT */
static __thread int tls_static = 8;

/* 9. TLS with protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 9;

/* 10. TLS with internal linkage */
static __thread int tls_internal = 10;

/* ===== HELPER FUNCTIONS ===== */

/* __attribute__((noinline)) ensures this function isn't optimized away */
__attribute__((noinline, used))
static void process_tls_variables(void) {
    /* Take addresses of all TLS variables - prevents dead code elimination */
    tls_addresses[0] = (void *)&tls_weak;
    tls_addresses[1] = (void *)&tls_hidden;
    tls_addresses[2] = (void *)&tls_common;
    tls_addresses[3] = (void *)&tls_external;
    tls_addresses[4] = (void *)&tls_imported;
    tls_addresses[5] = (void *)&tls_preserved;
    tls_addresses[6] = (void *)&tls_plain;
    tls_addresses[7] = (void *)&tls_static;
    tls_addresses[8] = (void *)&tls_protected;
    tls_addresses[9] = (void *)&tls_internal;
    
    /* Use TLS variables in computations */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common = 3;  /* Initialize the common variable */
    tls_values[3] = tls_external;
    tls_values[4] = tls_imported;
    tls_values[5] = tls_preserved;
    tls_values[6] = tls_plain;
    tls_values[7] = tls_static;
    tls_values[8] = tls_protected;
    tls_values[9] = tls_internal;
    
    /* Force compiler to keep all variables alive */
    for (int i = 0; i < 10; i++) {
        checksum_result += tls_values[i];
        checksum_result += (int)((uintptr_t)tls_addresses[i] & 0xFF);
    }
}

/* Another function that uses TLS in a different context */
__attribute__((noinline))
static void modify_tls_variables(void) {
    tls_weak++;
    tls_hidden *= 2;
    tls_common += 5;
    tls_external--;
    tls_imported = tls_imported * 3 + 1;
    tls_preserved ^= 0x55;
    tls_plain = tls_plain << 1;
    tls_static = tls_static >> 1;
    tls_protected += tls_protected;
    tls_internal = ~tls_internal;
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    printf("Testing emulated TLS coverage...\n");
    
    /* Initial processing */
    process_tls_variables();
    
    /* Modify and process again */
    modify_tls_variables();
    process_tls_variables();
    
    /* Force referencing of variables in main context */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum_result);
    
    /* Print individual values to ensure they're used */
    printf("Values: %d %d %d %d %d %d %d %d %d %d\n",
           tls_weak, tls_hidden, tls_common, tls_external,
           tls_imported, tls_preserved, tls_plain, tls_static,
           tls_protected, tls_internal);
    
    return 0;
}

/* ===== SECOND TRANSLATION UNIT SIMULATION ===== */
/* For full DECL_EXTERNAL testing, compile this separately:
 * 
 * File: tls_external_def.c
 * __thread int tls_external = 42;
 * 
 * Then link with: gcc -O2 -ftls-model=emulated tls_external_def.c test_emutls_coverage.c
 */
