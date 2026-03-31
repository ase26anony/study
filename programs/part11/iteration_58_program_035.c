/* tls_emulation_test.c
 * Tests GCC's emulated TLS initialization by creating thread-local variables
 * with diverse attributes that trigger the uncovered lines in tree-emutls.cc
 */

/* Force emulated TLS mode */
#ifndef __HAVE_TLS
#define __HAVE_TLS 0
#endif

#if __HAVE_TLS
#warning "Native TLS support detected - may not trigger emulated TLS paths"
#endif

/* Prevent dead code elimination */
#define USED __attribute__((used))
#define NOINLINE __attribute__((noinline))
#define RETAIN __attribute__((retain))

/* Global volatile array to prevent optimization */
volatile int g_results[10];
volatile void* g_addresses[10];

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;  /* No initializer */

/* 4. External TLS declaration - triggers TREE_PUBLIC and DECL_EXTERNAL */
extern __thread int tls_external;

/* 5. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used)) = 4;

/* 6. Public TLS with specific section */
__thread int tls_public USED = 5;

/* 7. Static TLS (non-public context) - affects DECL_CONTEXT */
static __thread int tls_static = 6;

/* 8. TLS in function scope - different DECL_CONTEXT */
void func_with_tls(void) {
    static __thread int tls_local_func = 7;
    g_addresses[7] = (void*)&tls_local_func;
}

/* 9. DLL Import simulation (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__attribute__((dllimport)) __thread int tls_imported;
#else
/* On non-Windows, use weak external to simulate similar behavior */
extern __thread int tls_imported __attribute__((weak));
#endif

/* ========== HELPER FUNCTIONS ========== */

NOINLINE static void use_tls_variables(void) {
    int idx = 0;
    
    /* Take addresses and use values of all TLS variables */
    g_addresses[idx++] = (void*)&tls_weak;
    g_results[0] = tls_weak * 2;
    
    g_addresses[idx++] = (void*)&tls_hidden;
    g_results[1] = tls_hidden + 1;
    
    g_addresses[idx++] = (void*)&tls_common;
    tls_common = 3;  /* Initialize the common variable */
    g_results[2] = tls_common;
    
    /* External TLS - will be defined below */
    g_addresses[idx++] = (void*)&tls_external;
    g_results[3] = tls_external;
    
    g_addresses[idx++] = (void*)&tls_preserved;
    g_results[4] = tls_preserved++;
    
    g_addresses[idx++] = (void*)&tls_public;
    g_results[5] = tls_public * tls_public;
    
    g_addresses[idx++] = (void*)&tls_static;
    g_results[6] = tls_static--;
    
    /* Imported TLS */
    g_addresses[idx++] = (void*)&tls_imported;
#ifdef _WIN32
    g_results[7] = tls_imported ? 1 : 0;
#else
    g_results[7] = &tls_imported ? 2 : 0;
#endif
    
    /* Force different addresses check */
    if (&tls_weak != &tls_hidden) {
        g_results[8] = 1;
    }
}

NOINLINE static int compute_tls_checksum(void) {
    int sum = 0;
    
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_external;
    sum += tls_preserved;
    sum += tls_public;
    sum += tls_static;
    
    /* Call function with local TLS */
    func_with_tls();
    
    return sum;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This would normally be in a separate file, but we define it here
 * with attribute to ensure it's treated as external during parsing */
__thread int tls_external __attribute__((externally_visible)) = 42;

/* For Windows/Mingw, we need a definition for the imported TLS */
#if defined(_WIN32) || defined(__MINGW32__)
__thread int tls_imported = 99;
#else
__thread int tls_imported = 100;
#endif

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int checksum;
    
    /* Initialize some TLS variables */
    tls_common = 33;
    tls_static = 66;
    
    /* Use TLS variables in helper function */
    use_tls_variables();
    
    /* Compute checksum using TLS variables */
    checksum = compute_tls_checksum();
    
    /* Print results to prevent optimization */
    volatile int result = checksum;
    
    /* Force reference to all TLS variables through pointer arithmetic */
    volatile void* ptr_array[] = {
        &tls_weak,
        &tls_hidden,
        &tls_common,
        &tls_external,
        &tls_preserved,
        &tls_public,
        &tls_static,
        &tls_imported
    };
    
    /* Simple output to ensure variables are used */
    if (result > 0) {
        return 0;  /* Success */
    }
    
    return 1;
}

/* ========== SECOND COMPILATION UNIT SIMULATION ========== */
/* To fully test external/DLL import scenarios, compile this separately:
 * 
 * File: tls_definitions.c
 * 
 * __thread int tls_external = 42;
 * 
 * #if defined(_WIN32) || defined(__MINGW32__)
 * __declspec(dllexport) __thread int tls_imported = 99;
 * #endif
 * 
 * Then link with: gcc -ftls-model=emulated tls_emulation_test.c tls_definitions.c
 */
