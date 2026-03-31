/* tls_emutls_test.c
 * Test program to cover GCC's emulated TLS initialization lines 295-304
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread tls_emutls_test.c -o tls_test
 * For 32-bit: gcc -O2 -m32 -ftls-model=emulated -fno-builtin tls_emutls_test.c -o tls_test32
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
volatile void* tls_addresses[10];
volatile int tls_values[10];

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

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

/* 6. TLS with used attribute - may influence DECL_PRESERVE_P */
__thread int tls_used __attribute__((used)) = 100;

/* 7. TLS inside a namespace-like static context (simulated) */
static int dummy_function() {
    /* Local static TLS - different DECL_CONTEXT */
    static __thread int tls_local_static = 7;
    return tls_local_static;
}

/* 8. DLL Import TLS simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar attribute on non-Windows */
__thread int tls_imported __attribute__((weak));
#endif

/* 9. TLS with retain attribute (GCC extension) - may influence DECL_PRESERVE_P */
#ifdef __GNUC__
__thread int tls_retained __attribute__((retain)) = 99;
#else
__thread int tls_retained = 99;
#endif

/* 10. TLS with section attribute - additional complexity */
__thread int tls_sectioned __attribute__((section(".tls_data"))) = 123;

/* ========== HELPER FUNCTIONS ========== */

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline)) 
static void process_tls_variables(void) {
    int idx = 0;
    
    /* Take addresses of all TLS variables - prevents dead code elimination */
    tls_addresses[idx++] = (void*)&tls_weak;
    tls_addresses[idx++] = (void*)&tls_hidden;
    tls_addresses[idx++] = (void*)&tls_common;
    tls_addresses[idx++] = (void*)&tls_external;
    tls_addresses[idx++] = (void*)&tls_public;
    tls_addresses[idx++] = (void*)&tls_used;
    tls_addresses[idx++] = (void*)&tls_imported;
    tls_addresses[idx++] = (void*)&tls_retained;
    tls_addresses[idx++] = (void*)&tls_sectioned;
    
    /* Use the values to force initialization */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = tls_external;
    tls_values[4] = tls_public;
    tls_values[5] = tls_used;
    tls_values[6] = tls_imported;
    tls_values[7] = tls_retained;
    tls_values[8] = tls_sectioned;
    
    /* Modify some values */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_public++;
    tls_used += 10;
}

/* Another function that uses TLS in a different context */
__attribute__((noinline))
static int compute_tls_checksum(void) {
    int sum = 0;
    
    /* Access all TLS variables */
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_external;
    sum += tls_public;
    sum += tls_used;
    sum += tls_imported;
    sum += tls_retained;
    sum += tls_sectioned;
    
    /* Force address comparisons */
    if (&tls_weak != &tls_hidden) {
        sum += 1000;
    }
    
    if (&tls_public != &tls_used) {
        sum += 2000;
    }
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int checksum;
    
    printf("Testing emulated TLS with diverse attributes\n");
    
    /* Initialize some TLS variables */
    tls_common = 5;
    tls_hidden = 10;
    
    /* Process TLS variables in helper function */
    process_tls_variables();
    
    /* Compute checksum using TLS variables */
    checksum = compute_tls_checksum();
    
    /* Print results to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    printf("TLS addresses differ: %s\n", 
           (&tls_weak != &tls_hidden) ? "yes" : "no");
    
    /* Print individual values */
    printf("tls_weak: %d at %p\n", tls_weak, (void*)&tls_weak);
    printf("tls_hidden: %d at %p\n", tls_hidden, (void*)&tls_hidden);
    printf("tls_common: %d at %p\n", tls_common, (void*)&tls_common);
    printf("tls_public: %d at %p\n", tls_public, (void*)&tls_public);
    printf("tls_used: %d at %p\n", tls_used, (void*)&tls_used);
    
    /* Force use of local static TLS */
    printf("Local static TLS: %d\n", dummy_function());
    
    return 0;
}

/* ========== SECOND TRANSLATION UNIT (if split compilation) ========== */
/* 
 * For external TLS variable definition (compile separately):
 * tls_external_def.c:
 * __thread int tls_external = 999;
 * 
 * Compile: gcc -c -ftls-model=emulated tls_external_def.c
 * Link: gcc -ftls-model=emulated tls_emutls_test.c tls_external_def.o -o tls_test
 */
