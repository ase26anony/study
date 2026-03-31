/* test-emutls-attributes.c
 * 
 * This program creates thread-local variables with various attributes
 * to trigger the property copying logic in GCC's emulated TLS implementation.
 * The goal is to cover lines 295-304 in tree-emutls.cc.
 */

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of TLS variable usage */
volatile void *volatile_global_ptr = NULL;
volatile int volatile_global_int = 0;

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
/* We'll define this in the same file for simplicity, but mark as extern first */
extern __thread int tls_external;

/* 5. DLL Import TLS simulation - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, simulate with visibility attributes */
__thread int tls_imported __attribute__((visibility("default")));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
/* Use 'used' attribute to suggest preservation */
__thread int tls_preserved __attribute__((used));

/* 7. Public TLS with initializer - ensures TREE_PUBLIC is set */
__thread int tls_initialized = 42;

/* 8. TLS in different context - affects DECL_CONTEXT */
static void function_with_tls(void) {
    /* Local static TLS - different context */
    static __thread int tls_in_function = 100;
    volatile_global_int += tls_in_function;
    tls_in_function++;
}

/* Define the external TLS variable */
__thread int tls_external = 123;

/* Helper function to use all TLS variables, marked noinline to prevent optimization */
__attribute__((noinline, used)) 
static void use_all_tls_variables(void) {
    /* Take addresses of all TLS variables */
    volatile void *addrs[] = {
        (void*)&tls_weak,
        (void*)&tls_hidden,
        (void*)&tls_common,
        (void*)&tls_external,
        (void*)&tls_imported,
        (void*)&tls_preserved,
        (void*)&tls_initialized
    };
    
    /* Store addresses to prevent optimization */
    for (int i = 0; i < sizeof(addrs)/sizeof(addrs[0]); i++) {
        volatile_global_ptr = addrs[i];
    }
    
    /* Use the values */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_external = 4;
    tls_imported = 5;
    tls_preserved = 6;
    tls_initialized = 7;
    
    /* Force computation with all TLS variables */
    volatile_global_int = 
        tls_weak + 
        tls_hidden + 
        tls_common + 
        tls_external + 
        tls_imported + 
        tls_preserved + 
        tls_initialized;
    
    /* Use TLS in function context */
    function_with_tls();
}

/* Another function that references TLS variables differently */
__attribute__((noinline))
static void reference_tls_addresses(void) {
    /* Force comparison of TLS addresses */
    if (&tls_weak != &tls_hidden) {
        volatile_global_int++;
    }
    if (&tls_common != &tls_external) {
        volatile_global_int++;
    }
}

/* Main function that orchestrates everything */
int main(void) {
    int checksum = 0;
    
    /* Initialize some TLS variables */
    tls_common = 10;
    tls_preserved = 20;
    
    /* Use all TLS variables through helper */
    use_all_tls_variables();
    
    /* Reference addresses */
    reference_tls_addresses();
    
    /* Compute a checksum using all TLS variables */
    checksum = 
        tls_weak +
        tls_hidden * 2 +
        tls_common * 3 +
        tls_external * 4 +
        tls_imported * 5 +
        tls_preserved * 6 +
        tls_initialized * 7;
    
    /* Print something to ensure execution */
    printf("TLS checksum: %d\n", checksum);
    printf("TLS addresses differ: %s\n", 
           (&tls_weak != &tls_hidden) ? "yes" : "no");
    
    /* Force use of volatile globals */
    printf("Volatile global int: %d\n", volatile_global_int);
    
    return checksum != 0 ? 0 : 1;
}

/* Additional compilation unit simulation */
#ifdef COMPILE_SECOND_PART
/* This would normally be in a separate file for DLL import testing */
__declspec(dllexport) __thread int tls_imported = 999;

/* Another external TLS definition */
__thread int another_external_tls __attribute__((weak, visibility("default")));
#endif
