/* test-emutls-coverage.c
 * Comprehensive test to cover GCC's emulated TLS property copying logic
 * Specifically targets lines 295-304 in tree-emutls.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode if possible */
#if defined(__GNUC__) && !defined(__HAVE_TLS)
#pragma GCC tls_model emulated
#endif

/* Prevent optimization of TLS variable usage */
volatile void* volatile_global_ptr;
volatile int volatile_global_int;

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may set DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - will be defined in another TU or later */
extern __thread int tls_external;

/* 5. Public TLS variable with used attribute - affects TREE_PUBLIC and DECL_EXTERNAL */
__thread int tls_public __attribute__((used));

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 8. TLS variable with multiple attributes */
__thread int tls_multi __attribute__((weak, visibility("default")));

/* Windows-specific DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__declspec(dllimport) __thread int tls_imported;
#else
/* For non-Windows, use weak external to simulate similar behavior */
extern __thread int tls_imported __attribute__((weak));
#endif

/* ===== HELPER FUNCTIONS ===== */

/* Force noinline to ensure TLS variables are processed */
__attribute__((noinline, used))
static void use_tls_variables(void) {
    /* Take addresses of all TLS variables to prevent optimization */
    volatile void* addrs[] = {
        (void*)&tls_weak,
        (void*)&tls_hidden,
        (void*)&tls_common,
        (void*)&tls_external,
        (void*)&tls_public,
        (void*)&tls_preserved,
        (void*)&tls_init,
        (void*)&tls_multi,
        (void*)&tls_imported
    };
    
    volatile_global_ptr = addrs[0];
    
    /* Use TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_public = 4;
    tls_preserved = 5;
    tls_multi = tls_weak + tls_hidden;
    
    /* Create dependency chain */
    int sum = tls_weak + tls_hidden + tls_common + tls_public + 
              tls_preserved + tls_init + tls_multi;
    
    volatile_global_int = sum;
    
    /* Force compiler to consider all TLS variables as used */
    if (&tls_weak != &tls_hidden) {
        volatile_global_int++;
    }
}

/* Another function to create different DECL_CONTEXT */
__attribute__((noinline))
static void nested_tls_usage(void) {
    /* Static TLS inside function - different context */
    static __thread int tls_static_func = 100;
    
    tls_static_func++;
    volatile_global_int += tls_static_func;
    
    /* Use external TLS */
    if (&tls_external != NULL) {
        volatile_global_int += 1;
    }
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    /* Initialize some TLS variables */
    tls_common = 10;
    tls_public = 20;
    tls_preserved = 30;
    
    /* Use TLS variables in helper functions */
    use_tls_variables();
    nested_tls_usage();
    
    /* Force reference to all TLS variables in main */
    volatile int* ptrs[] = {
        &tls_weak,
        &tls_hidden,
        &tls_common,
        &tls_external,
        &tls_public,
        &tls_preserved,
        &tls_init,
        &tls_multi,
        &tls_imported
    };
    
    /* Compute checksum from TLS addresses */
    uintptr_t checksum = 0;
    for (int i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
        checksum += (uintptr_t)ptrs[i];
    }
    
    /* Use checksum to prevent dead code elimination */
    volatile_global_int += (int)(checksum & 0xFFFFFFFF);
    
    /* Print something to ensure execution */
    printf("TLS test executed. Checksum: %lu\n", (unsigned long)checksum);
    printf("tls_init = %d\n", tls_init);
    printf("volatile_global_int = %d\n", volatile_global_int);
    
    return 0;
}

/* ===== SECOND TRANSLATION UNIT SIMULATION ===== */
/* In a real multi-TU test, this would be in a separate file */
/* Uncomment for multi-TU testing */

/*
// File: tls-definitions.c
__thread int tls_external = 123;

#ifdef _WIN32
__declspec(dllexport) __thread int tls_imported = 456;
#elif defined(__MINGW32__)
__declspec(dllexport) __thread int tls_imported = 456;
#else
__thread int tls_imported = 456;
#endif
*/
