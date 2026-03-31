/* Main test file for emulated TLS coverage */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode */
#ifdef __GNUC__
#pragma GCC tls_model emulated
#endif

/* Global volatile array to prevent optimization */
volatile void* tls_addresses[10];
volatile int tls_values[10];

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 6. Public TLS with initializer */
__thread int tls_public = 42;

/* 7. Static TLS (non-public context) - affects DECL_CONTEXT */
static __thread int tls_static = 100;

/* 8. Weak alias TLS */
__thread int tls_original = 200;
extern __thread int tls_alias __attribute__((weak, alias("tls_original")));

/* Windows-specific DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar concept on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif

/* ===== HELPER FUNCTIONS ===== */

/* Force noinline to ensure TLS variables are processed */
__attribute__((noinline, used)) 
static void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = &tls_weak;
    tls_addresses[1] = &tls_hidden;
    tls_addresses[2] = &tls_common;
    tls_addresses[3] = &tls_external;
    tls_addresses[4] = &tls_preserved;
    tls_addresses[5] = &tls_public;
    tls_addresses[6] = &tls_static;
    tls_addresses[7] = &tls_original;
    tls_addresses[8] = &tls_alias;
    tls_addresses[9] = &tls_imported;
    
    /* Use TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = 4;
    tls_static = 5;
    
    /* Compute checksum using TLS values */
    int sum = tls_weak + tls_hidden + tls_common + tls_preserved + tls_static;
    tls_values[0] = sum;
    
    /* Force different addresses check */
    if (&tls_weak != &tls_hidden) {
        tls_values[1] = 1;
    }
    
    /* Use external TLS */
    tls_values[2] = tls_external;
    
    /* Use imported TLS */
    tls_values[3] = tls_imported;
}

/* Another function with different context */
__attribute__((noinline))
void modify_tls_from_function(void) {
    static __thread int function_local_tls = 0;
    function_local_tls++;
    tls_values[4] = function_local_tls;
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    /* Initialize some TLS variables */
    tls_common = 10;
    tls_preserved = 20;
    
    /* Use TLS variables */
    use_tls_variables();
    modify_tls_from_function();
    
    /* Force TLS usage in main */
    volatile int* ptr1 = &tls_weak;
    volatile int* ptr2 = &tls_hidden;
    
    /* Compute and print result to prevent dead code elimination */
    int result = tls_weak + tls_hidden + tls_common + tls_public + tls_static;
    
    /* Print addresses to ensure TLS variables are referenced */
    printf("TLS addresses:\n");
    printf("  tls_weak: %p\n", (void*)&tls_weak);
    printf("  tls_hidden: %p\n", (void*)&tls_hidden);
    printf("  tls_common: %p\n", (void*)&tls_common);
    printf("  tls_public: %p\n", (void*)&tls_public);
    printf("  tls_static: %p\n", (void*)&tls_static);
    
    printf("Result: %d\n", result);
    printf("External TLS value: %d\n", tls_external);
    
    /* Check if we're using emulated TLS */
#ifdef __HAVE_TLS
    printf("Native TLS support: YES\n");
#else
    printf("Native TLS support: NO (using emulated TLS)\n");
#endif
    
    return 0;
}
