/* tls_emutls_test.c
 * Test program to cover GCC's emulated TLS initialization lines 295-304
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread tls_emutls_test.c -o tls_test
 * For 32-bit: gcc -O2 -m32 -ftls-model=emulated -fno-builtin tls_emutls_test.c -o tls_test32
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if we're in emulated mode */
#ifndef __HAVE_TLS
#define EMULATED_TLS 1
#else
/* Still try to force emulated mode with compiler flag */
#define EMULATED_TLS 1
#endif

/* Global volatile array to prevent optimization */
volatile void* tls_addresses[10];
volatile int tls_values[10];

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable - tentative definition, may set DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - will be defined later, sets DECL_EXTERNAL */
extern __thread int tls_external;

/* 5. Public TLS with used attribute - ensures TREE_PUBLIC and preservation */
__thread int tls_public __attribute__((used));

/* 6. TLS with initialization - not BSS, ensures proper handling */
__thread int tls_init = 42;

/* 7. TLS in different linkage contexts */
static __thread int tls_static;  /* Internal linkage */

/* 8. DLL Import simulation (for Windows/MinGW) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, simulate with external declaration */
extern __thread int tls_imported;
#endif

/* 9. TLS with preserve attribute (using retain if available) */
#if __has_attribute(retain)
__thread int tls_preserved __attribute__((used, retain));
#elif __has_attribute(noinline)
/* Use in noinline function as alternative */
__thread int tls_preserved __attribute__((used));
#else
__thread int tls_preserved __attribute__((used));
#endif

/* 10. TLS in namespace/struct context (affects DECL_CONTEXT) */
struct Container {
    static __thread int member_tls;  /* C++ style, but in C we simulate */
};
/* Simulate member TLS with prefix */
__thread int container_member_tls;

/* ========== HELPER FUNCTIONS ========== */

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline)) 
static void process_tls_variables(void) {
    /* Take addresses of all TLS variables - prevents dead code elimination */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_external;
    tls_addresses[4] = (void*)&tls_public;
    tls_addresses[5] = (void*)&tls_init;
    tls_addresses[6] = (void*)&tls_static;
    tls_addresses[7] = (void*)&tls_imported;
    tls_addresses[8] = (void*)&tls_preserved;
    tls_addresses[9] = (void*)&container_member_tls;
    
    /* Use the TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    /* tls_external defined elsewhere */
    tls_public = 5;
    tls_init = tls_init * 2;  /* Modify initialized TLS */
    tls_static = 7;
    /* tls_imported might be external */
    tls_preserved = 9;
    container_member_tls = 10;
    
    /* Store values to prevent optimization */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = 0;  /* tls_external placeholder */
    tls_values[4] = tls_public;
    tls_values[5] = tls_init;
    tls_values[6] = tls_static;
    tls_values[7] = 0;  /* tls_imported placeholder */
    tls_values[8] = tls_preserved;
    tls_values[9] = container_member_tls;
}

/* Another function to ensure TLS is used in multiple contexts */
__attribute__((noinline))
static int compute_tls_checksum(void) {
    int sum = 0;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_public;
    sum += tls_init;
    sum += tls_static;
    sum += tls_preserved;
    sum += container_member_tls;
    return sum;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This would normally be in a separate file, but we include it here
 * with a guard to simulate multi-TU scenario */
#ifdef DEFINE_EXTERNAL_TLS
__thread int tls_external = 100;
__thread int tls_imported = 200;
#else
/* Just declarations - the definitions would be elsewhere */
#endif

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int checksum;
    
    printf("Testing emulated TLS with various attributes...\n");
    
    /* Initialize some TLS variables */
    tls_common = 123;
    tls_public = 456;
    
    /* Process all TLS variables */
    process_tls_variables();
    
    /* Force referencing of variables in main */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Compute and print checksum */
    checksum = compute_tls_checksum();
    printf("TLS checksum: %d\n", checksum);
    
    /* Print some addresses to ensure variables are used */
    printf("tls_weak address: %p\n", (void*)&tls_weak);
    printf("tls_hidden address: %p\n", (void*)&tls_hidden);
    printf("tls_init value: %d\n", tls_init);
    
    /* Check if we're likely using emulated TLS */
#ifdef EMULATED_TLS
    printf("Compiled with emulated TLS support\n");
#endif
    
    return 0;
}

/* ========== SECOND FILE SIMULATION ========== */
/* To properly test external/DLL import, compile this separately:
 * 
 * File 1: tls_external_def.c
 * ------
 * #define DEFINE_EXTERNAL_TLS
 * #include "tls_emutls_test.c"
 * 
 * File 2: tls_main.c  
 * ------
 * #include "tls_emutls_test.c"
 * 
 * Then compile and link:
 * gcc -O2 -ftls-model=emulated -c tls_external_def.c -o tls_def.o
 * gcc -O2 -ftls-model=emulated -c tls_main.c -o tls_main.o
 * gcc -ftls-model=emulated tls_def.o tls_main.o -o tls_test_multi
 */
