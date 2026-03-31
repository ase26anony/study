/* tls_main.c - Main file with various TLS variable declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation flags */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O0")
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* Public TLS with explicit visibility and used attribute */
__thread int tls_public __attribute__((used, visibility("default"))) = 42;

/* Weak TLS definition - can be overridden */
__thread int tls_weak __attribute__((weak)) = 100;

/* Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_imported __attribute__((dllimport));
#endif

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* TLS with specific context (inside static function) */
static void static_function_context(void) {
    /* TLS inside function context - different DECL_CONTEXT */
    static __thread int tls_in_function = 300;
    tls_in_function++;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&tls_in_function));
}

/* ========== HELPER FUNCTIONS ========== */

/* Force address taking without side effects */
#define FORCE_USE(var) asm volatile("" : : "r"(&(var)))

/* Checksum function to ensure all TLS vars are used */
static uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    sum += tls_external;
    sum += tls_external_weak;
    sum += tls_external_hidden;
    
    /* Access imported TLS if available */
    if (&tls_imported != NULL) {
        sum += tls_imported;
    }
    
    static_function_context();
    
    return sum;
}

/* Function that modifies TLS variables */
void modify_tls_vars(void) {
    tls_public += 1;
    tls_weak *= 2;
    tls_common = tls_public + tls_weak;
    tls_hidden -= 5;
    
    if (&tls_external != NULL) {
        tls_external++;
    }
    
    /* Force usage of all variables */
    FORCE_USE(tls_public);
    FORCE_USE(tls_weak);
    FORCE_USE(tls_common);
    FORCE_USE(tls_hidden);
    FORCE_USE(tls_imported);
}

/* Thread-like behavior simulation */
void simulate_thread_behavior(int thread_id) {
    /* Each "thread" gets unique TLS values */
    tls_public = thread_id * 1000;
    tls_common = thread_id;
    tls_hidden = thread_id * 2000;
    
    /* Use conditional logic with TLS */
    if (tls_public > 5000) {
        tls_weak = 999;
    } else {
        tls_weak = 111;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    uint32_t checksums[3] = {0};
    
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Simulate different thread contexts */
    for (int i = 0; i < 3; i++) {
        simulate_thread_behavior(i + 1);
        modify_tls_vars();
        checksums[i] = tls_checksum();
        
        printf("Thread %d TLS checksum: %u\n", i + 1, checksums[i]);
        printf("  tls_public=%d, tls_weak=%d, tls_common=%d\n",
               tls_public, tls_weak, tls_common);
    }
    
    /* Verify TLS variables maintain separate "thread" values */
    if (checksums[0] != checksums[1] && checksums[1] != checksums[2]) {
        printf("TLS variables show thread-local behavior\n");
    }
    
    /* Additional test: Mix of static and extern usage */
    {
        /* Force external TLS usage with different patterns */
        volatile int* ptrs[] = {
            &tls_external,
            &tls_external_weak,
            &tls_external_hidden,
            &tls_public,
            &tls_weak
        };
        
        for (size_t i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
            if (ptrs[i] != NULL) {
                *ptrs[i] += i;
            }
        }
    }
    
    return 0;
}
