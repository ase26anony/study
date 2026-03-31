/* test_emutls_coverage.c
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 -o test_emutls test_emutls_coverage.c
 * Or for Windows: gcc -O2 -ftls-model=emulated -fno-builtin -D_WIN32 -o test_emutls test_emutls_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is unavailable */
#ifndef __HAVE_TLS
#define USE_EMULATED_TLS 1
#endif

/* Global volatile array to prevent optimization */
volatile uintptr_t tls_addresses[10];

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers TREE_PUBLIC and DECL_EXTERNAL */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, use weak import simulation */
__thread int tls_imported __attribute__((weak));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 8. TLS in different linkage contexts */
static __thread int tls_static;  /* Internal linkage */

/* 9. TLS with section attribute */
__thread int tls_sectioned __attribute__((section(".tls_data")));

/* ========== HELPER FUNCTIONS ========== */

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline, used))
static void process_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = (uintptr_t)&tls_weak;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    tls_addresses[2] = (uintptr_t)&tls_common;
    tls_addresses[3] = (uintptr_t)&tls_external;
    tls_addresses[4] = (uintptr_t)&tls_imported;
    tls_addresses[5] = (uintptr_t)&tls_preserved;
    tls_addresses[6] = (uintptr_t)&tls_init;
    tls_addresses[7] = (uintptr_t)&tls_static;
    tls_addresses[8] = (uintptr_t)&tls_sectioned;
    
    /* Use the TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    /* tls_external is defined elsewhere */
    tls_imported = 5;
    tls_preserved = 6;
    tls_init = 7;
    tls_static = 8;
    tls_sectioned = 9;
    
    /* Compute checksum using TLS values */
    int checksum = tls_weak + tls_hidden + tls_common + tls_imported + 
                   tls_preserved + tls_init + tls_static + tls_sectioned;
    
    tls_addresses[9] = (uintptr_t)checksum;
}

/* Function to create non-global DECL_CONTEXT */
__attribute__((noinline))
static void function_with_local_tls(void) {
    /* TLS with function scope context */
    static __thread int local_func_tls = 100;
    tls_addresses[0] += local_func_tls;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize some TLS variables */
    tls_common = 10;
    tls_preserved = 20;
    
    /* Process all TLS variables */
    process_tls_variables();
    
    /* Use TLS in different context */
    function_with_local_tls();
    
    /* Force referencing of variables through conditional checks */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print checksum to ensure TLS is used */
    printf("TLS checksum: %lu\n", (unsigned long)tls_addresses[9]);
    
    /* Print addresses to prevent optimization */
    for (int i = 0; i < 9; i++) {
        printf("TLS address %d: 0x%lx\n", i, (unsigned long)tls_addresses[i]);
    }
    
    return 0;
}

/* ========== SECOND TRANSLATION UNIT (if compiling separately) ========== */
/* 
 * For multi-file compilation to test DECL_EXTERNAL fully:
 * File2.c:
 * __thread int tls_external = 123;
 * 
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -c test_emutls_coverage.c -o test1.o
 *               gcc -O2 -ftls-model=emulated -fno-builtin -c file2.c -o test2.o
 *               gcc -m32 -o test_emutls test1.o test2.o -pthread
 */
