/* test-emutls-coverage.c
 * Comprehensive test to cover emulated TLS property copying in tree-emutls.cc
 * Lines 295-304: DECL_PRESERVE_P, DECL_CONTEXT, TREE_USED, TREE_PUBLIC,
 * DECL_EXTERNAL, DECL_COMMON, DECL_WEAK, DECL_VISIBILITY,
 * DECL_VISIBILITY_SPECIFIED, DECL_DLLIMPORT_P
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is not available */
#ifndef __HAVE_TLS
#define USE_EMULATED_TLS 1
#else
/* Try to force emulated TLS anyway with compiler flags */
#define USE_EMULATED_TLS 1
#endif

/* Prevent optimization of TLS variable usage */
volatile void *volatile tls_addresses[10];
volatile int volatile tls_values[10];
int checksum = 0;

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));
__thread int tls_internal __attribute__((visibility("internal")));
__thread int tls_protected __attribute__((visibility("protected")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__attribute__((dllimport)) __thread int tls_imported;
#else
/* Simulate with visibility on non-Windows */
__thread int tls_imported __attribute__((visibility("default")));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. Public TLS with initializer - triggers TREE_PUBLIC */
__thread int tls_public = 42;

/* 8. Used TLS variable - triggers TREE_USED */
__thread int tls_used __attribute__((used));

/* 9. TLS in different contexts - affects DECL_CONTEXT */
static void function_with_tls(void) {
    /* TLS with function scope context */
    static __thread int tls_function_scope = 100;
    tls_addresses[8] = (void *)&tls_function_scope;
    tls_values[8] = tls_function_scope++;
}

/* 10. Complex TLS with multiple attributes */
__thread int tls_complex __attribute__((weak, visibility("hidden"), used));

/* ========== HELPER FUNCTIONS ========== */

/* __attribute__((noinline)) ensures function isn't optimized away */
__attribute__((noinline, used))
static void use_tls_variables(void) {
    int idx = 0;
    
    /* Take addresses and use values of all TLS variables */
    
    /* 1. Weak TLS */
    tls_weak = 1;
    tls_addresses[idx] = (void *)&tls_weak;
    tls_values[idx] = tls_weak;
    idx++;
    
    /* 2. Hidden TLS */
    tls_hidden = 2;
    tls_addresses[idx] = (void *)&tls_hidden;
    tls_values[idx] = tls_hidden;
    idx++;
    
    /* 3. Internal TLS */
    tls_internal = 3;
    tls_addresses[idx] = (void *)&tls_internal;
    tls_values[idx] = tls_internal;
    idx++;
    
    /* 4. Protected TLS */
    tls_protected = 4;
    tls_addresses[idx] = (void *)&tls_protected;
    tls_values[idx] = tls_protected;
    idx++;
    
    /* 5. Common TLS */
    tls_common = 5;
    tls_addresses[idx] = (void *)&tls_common;
    tls_values[idx] = tls_common;
    idx++;
    
    /* 6. External TLS (will be defined in another TU if needed) */
    tls_addresses[idx] = (void *)&tls_external;
    tls_values[idx] = 0; /* Don't write to external */
    idx++;
    
    /* 7. Imported TLS */
    tls_imported = 7;
    tls_addresses[idx] = (void *)&tls_imported;
    tls_values[idx] = tls_imported;
    idx++;
    
    /* 8. Preserved TLS */
    tls_preserved = 8;
    tls_addresses[idx] = (void *)&tls_preserved;
    tls_values[idx] = tls_preserved;
    idx++;
    
    /* 9. Public TLS */
    tls_public++;
    tls_addresses[idx] = (void *)&tls_public;
    tls_values[idx] = tls_public;
    idx++;
    
    /* 10. Used TLS */
    tls_used = 10;
    tls_addresses[idx] = (void *)&tls_used;
    tls_values[idx] = tls_used;
    idx++;
    
    /* 11. Complex TLS */
    tls_complex = 11;
    tls_addresses[idx] = (void *)&tls_complex;
    tls_values[idx] = tls_complex;
    
    /* Call function with scope-local TLS */
    function_with_tls();
}

/* Another noinline function to ensure TLS variables are referenced multiple times */
__attribute__((noinline, used))
static void compute_tls_checksum(void) {
    int sum = 0;
    
    /* Force computation using all TLS variables */
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_internal;
    sum += tls_protected;
    sum += tls_common;
    /* tls_external is external, don't read */
    sum += tls_imported;
    sum += tls_preserved;
    sum += tls_public;
    sum += tls_used;
    sum += tls_complex;
    
    /* Store in global to prevent optimization */
    checksum = sum;
    
    /* Force comparison of TLS addresses (prevents optimization) */
    if (&tls_weak != &tls_hidden) {
        /* This condition is always true, but compiler doesn't know */
        checksum += 1;
    }
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    printf("Testing emulated TLS coverage...\n");
    
    /* Use TLS variables in helper functions */
    use_tls_variables();
    compute_tls_checksum();
    
    /* Print some results to ensure variables are used */
    printf("TLS weak address: %p\n", (void *)&tls_weak);
    printf("TLS hidden address: %p\n", (void *)&tls_hidden);
    printf("TLS common value: %d\n", tls_common);
    printf("TLS public value: %d\n", tls_public);
    printf("Checksum: %d\n", checksum);
    
    /* Force reference to external TLS symbol */
    printf("External TLS address: %p\n", (void *)&tls_external);
    
    return 0;
}
