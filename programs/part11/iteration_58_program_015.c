/* test-emutls-coverage.c
 * Comprehensive test for GCC emulated TLS initialization coverage
 * Targets lines 295-304 in tree-emutls.cc
 */

/* Force emulated TLS mode */
#if defined(__GNUC__) && !defined(__HAVE_TLS)
/* Already using emulated TLS */
#elif defined(__GNUC__)
/* Try to force emulated TLS */
#pragma GCC optimize("tls-model=emulated")
#endif

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of TLS variable usage */
volatile void *volatile_global_ptr;
volatile int volatile_global_int;

/* Helper to prevent dead code elimination */
__attribute__((noinline, used))
static void use_pointer(const void *p) {
    volatile_global_ptr = p;
}

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may set DECL_COMMON */
__thread int tls_common;

/* 4. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 5. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 6. Used attribute - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. TLS variable with both used and visibility attributes */
__thread int tls_complex __attribute__((used, visibility("default")));

/* 8. Static TLS variable - different linkage */
static __thread int tls_static;

/* 9. TLS variable inside a function - different DECL_CONTEXT */
static void function_with_tls(void) {
    static __thread int tls_local_func = 100;
    volatile_global_int += tls_local_func;
}

/* 10. DLL Import simulation (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__attribute__((dllimport)) __thread int tls_imported;
#else
/* For non-Windows, create a weak external to simulate similar behavior */
extern __thread int tls_imported __attribute__((weak));
#endif

/* ===== TLS USAGE FUNCTIONS ===== */

/* Function that uses all TLS variables to ensure they're processed */
__attribute__((noinline, used))
static void use_all_tls_variables(void) {
    /* Take addresses to force TLS instantiation */
    int *ptrs[10];
    int i = 0;
    
    ptrs[i++] = &tls_weak;
    ptrs[i++] = &tls_hidden;
    ptrs[i++] = &tls_common;
    ptrs[i++] = &tls_init;
    ptrs[i++] = &tls_preserved;
    ptrs[i++] = &tls_complex;
    ptrs[i++] = &tls_static;
    
    /* Use the external TLS variable */
    if (&tls_external != NULL) {
        ptrs[i++] = &tls_external;
    }
    
    /* Use the imported TLS variable */
    if (&tls_imported != NULL) {
        ptrs[i++] = &tls_imported;
    }
    
    /* Modify TLS variables */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = 4;
    tls_complex = 5;
    tls_static = 6;
    
    /* Force computation using TLS values */
    volatile_global_int = tls_weak + tls_hidden + tls_common + 
                         tls_init + tls_preserved + tls_complex + tls_static;
    
    /* Use function-local TLS */
    function_with_tls();
    
    /* Store addresses to prevent optimization */
    for (int j = 0; j < i; j++) {
        use_pointer(ptrs[j]);
    }
}

/* Another function that references TLS in a different way */
__attribute__((noinline))
static int compute_tls_checksum(void) {
    int sum = 0;
    
    /* Force reads of all TLS variables */
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_init;
    sum += tls_preserved;
    sum += tls_complex;
    sum += tls_static;
    
    /* Reference external TLS */
    extern __thread int tls_external;
    if (&tls_external != NULL) {
        sum += 1; /* Placeholder for actual value */
    }
    
    return sum;
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    printf("Testing emulated TLS coverage...\n");
    
    /* Force TLS variable initialization by using them */
    use_all_tls_variables();
    
    /* Compute and print checksum */
    int checksum = compute_tls_checksum();
    printf("TLS checksum: %d\n", checksum);
    
    /* Force comparison of TLS addresses */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print individual TLS addresses to ensure they're used */
    printf("TLS addresses:\n");
    printf("  tls_weak: %p\n", (void*)&tls_weak);
    printf("  tls_hidden: %p\n", (void*)&tls_hidden);
    printf("  tls_common: %p\n", (void*)&tls_common);
    printf("  tls_init: %p\n", (void*)&tls_init);
    printf("  tls_preserved: %p\n", (void*)&tls_preserved);
    printf("  tls_complex: %p\n", (void*)&tls_complex);
    printf("  tls_static: %p\n", (void*)&tls_static);
    
    return 0;
}

/* ===== SECOND TRANSLATION UNIT (if split compilation is used) ===== */
/* 
 * To test DECL_EXTERNAL and DLL import properly, compile this separately:
 * 
 * // tls-definitions.c
 * __thread int tls_external = 123;
 * __thread int tls_imported = 456;
 * 
 * Then link with: gcc -ftls-model=emulated test-emutls-coverage.c tls-definitions.c
 */
