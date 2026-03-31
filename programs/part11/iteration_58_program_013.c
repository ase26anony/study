/* Main test file for emulated TLS attribute copying coverage */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode */
#if defined(__GNUC__) && !defined(__HAVE_TLS)
#pragma message "Using emulated TLS mode"
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

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

/* 6. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 7. Static TLS variable with local linkage */
static __thread int tls_static = 100;

/* 8. TLS variable in a namespace-like context (using struct) */
struct Container {
    static __thread int member_tls;  /* C++ style, but we'll handle in C++ file */
};

/* Windows-specific DLL import TLS */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar concept for non-Windows */
extern __thread int tls_imported;
#endif

/* ========== HELPER FUNCTIONS ========== */

/* Prevent optimization of TLS variable usage */
volatile uintptr_t tls_addresses[10];
volatile int tls_values[10];

/* Noinline function to ensure TLS variables are processed */
__attribute__((noinline, used))
void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = (uintptr_t)&tls_weak;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    tls_addresses[2] = (uintptr_t)&tls_common;
    tls_addresses[3] = (uintptr_t)&tls_external;
    tls_addresses[4] = (uintptr_t)&tls_preserved;
    tls_addresses[5] = (uintptr_t)&tls_init;
    tls_addresses[6] = (uintptr_t)&tls_static;
    
    /* Use TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = 4;
    
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = tls_external;
    tls_values[4] = tls_preserved;
    tls_values[5] = tls_init;
    tls_values[6] = tls_static;
    
    /* Force compiler to consider all TLS variables as used */
    if (&tls_weak == &tls_hidden) {
        /* This should never happen, but prevents dead code elimination */
        tls_values[7] = 1;
    }
}

/* Another function that uses TLS in a different context */
__attribute__((noinline))
int compute_tls_checksum(void) {
    int sum = 0;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_external;
    sum += tls_preserved;
    sum += tls_init;
    sum += tls_static;
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize some TLS variables */
    tls_common = 10;
    tls_preserved = 20;
    
    /* Use TLS variables in helper function */
    use_tls_variables();
    
    /* Compute checksum to ensure all TLS variables are used */
    int checksum = compute_tls_checksum();
    
    /* Print results to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    printf("TLS addresses differ: %d\n", 
           (&tls_weak != &tls_hidden) &&
           (&tls_hidden != &tls_common));
    
    /* Force reference to imported TLS */
#ifdef _WIN32
    printf("Imported TLS address: %p\n", (void*)&tls_imported);
#endif
    
    return 0;
}
