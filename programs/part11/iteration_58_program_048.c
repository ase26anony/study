/* test_emutls_coverage.c
 * Comprehensive test to cover emulated TLS attribute copying in GCC's tree-emutls.cc
 * Lines 295-304: DECL_PRESERVE_P, DECL_CONTEXT, TREE_USED, TREE_PUBLIC,
 * DECL_EXTERNAL, DECL_COMMON, DECL_WEAK, DECL_VISIBILITY, 
 * DECL_VISIBILITY_SPECIFIED, DECL_DLLIMPORT_P
 */

/* Force emulated TLS mode if supported by compiler */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC tls_model emulated
#endif

/* For Windows DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT
#define DLL_EXPORT
#endif

/* Prevent optimization of TLS variable usage */
#define USE_VAR(var) do { \
    volatile void *volatile_ptr = (volatile void*)&(var); \
    (void)volatile_ptr; \
} while(0)

/* Helper to force variable preservation */
#define FORCE_USED __attribute__((used, noinline))

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;  /* No initializer for common linkage */

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
DLL_IMPORT __thread int tls_imported;

/* 6. Preserved TLS variable with used attribute - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used)) = 6;

/* 7. Public TLS with explicit initialization - triggers TREE_PUBLIC */
__thread int tls_public = 7;

/* 8. Static TLS (non-public) for DECL_CONTEXT testing */
static __thread int tls_static = 8;

/* 9. TLS in function scope for different DECL_CONTEXT */
static void function_with_tls(void) {
    static __thread int tls_function_scope = 9;
    USE_VAR(tls_function_scope);
}

/* 10. TLS with section attribute to ensure it's processed */
__thread int tls_section __attribute__((section(".tls_test"))) = 10;

/* ========== EXTERNAL TLS DEFINITION (for multi-TU simulation) ========== */
/* In a real multi-TU test, this would be in a separate file */
#ifdef DEFINE_EXTERNAL_TLS
__thread int tls_external = 42;
DLL_EXPORT __thread int tls_imported = 43;
#endif

/* ========== HELPER FUNCTIONS TO USE TLS VARIABLES ========== */

/* Force this function to not be inlined to ensure TLS variables are fully processed */
static FORCE_USED void use_all_tls_variables(void) {
    /* Take addresses and use all TLS variables */
    volatile int *ptrs[10];
    
    ptrs[0] = &tls_weak;
    ptrs[1] = &tls_hidden;
    ptrs[2] = &tls_common;
    
    /* External TLS - will be resolved at link time */
    #ifndef DEFINE_EXTERNAL_TLS
    extern __thread int tls_external;
    ptrs[3] = &tls_external;
    #else
    ptrs[3] = &tls_external;
    #endif
    
    ptrs[4] = &tls_preserved;
    ptrs[5] = &tls_public;
    ptrs[6] = &tls_static;
    ptrs[7] = &tls_section;
    
    /* Modify TLS variables to ensure they're not optimized away */
    tls_weak += 1;
    tls_hidden *= 2;
    tls_common = 100;
    tls_preserved ^= 0xFF;
    tls_public = tls_public > 0 ? tls_public : 1;
    tls_static++;
    tls_section = (tls_section << 1) | 1;
    
    /* Force computation using all TLS variables */
    volatile int checksum = 0;
    for (int i = 0; i < 8; i++) {
        if (ptrs[i]) {
            checksum += *ptrs[i];
        }
    }
    
    /* Store result in global volatile to prevent optimization */
    extern volatile int global_checksum;
    global_checksum = checksum;
}

/* Another function that uses TLS in a different context */
static __attribute__((noinline)) void nested_tls_usage(void) {
    __thread int tls_nested = 123;
    static __thread int tls_static_nested = 456;
    
    tls_nested += tls_weak;
    tls_static_nested += tls_hidden;
    
    USE_VAR(tls_nested);
    USE_VAR(tls_static_nested);
}

/* ========== GLOBAL VOLATILE FOR OPTIMIZATION PREVENTION ========== */
volatile int global_checksum = 0;

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Initialize common TLS variable */
    tls_common = 3;
    
    /* Use all TLS variables through helper */
    use_all_tls_variables();
    
    /* Use TLS in nested context */
    nested_tls_usage();
    
    /* Force TLS variable usage in main */
    function_with_tls();
    
    /* Create observable side effects with TLS variables */
    volatile int result = 0;
    result += tls_weak;
    result += tls_hidden;
    result += tls_common;
    result += tls_preserved;
    result += tls_public;
    result += tls_static;
    result += tls_section;
    
    /* Force compiler to generate emulated TLS structures */
    if (&tls_weak != &tls_hidden) {
        result |= 0x80000000;
    }
    
    /* Check if we're using emulated TLS at runtime */
    #ifndef __HAVE_TLS
    result |= 0x40000000;
    #endif
    
    /* Return checksum to prevent optimization */
    return (result ^ global_checksum) & 0xFF;
}

/* ========== SECOND TRANSLATION UNIT SIMULATION ========== */
/* For a true multi-TU test, compile this separately: */
#ifdef COMPILE_SECOND_TU
__thread int tls_external = 42;

#ifdef _WIN32
__declspec(dllexport) __thread int tls_imported = 43;
#endif

/* Function that uses the TLS variables */
int use_external_tls(void) {
    return tls_external + tls_imported;
}
#endif
