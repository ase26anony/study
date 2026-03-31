/* test_emutls_coverage.c
 * 
 * This program creates multiple thread-local variables with different
 * attributes to trigger GCC's emulated TLS initialization logic,
 * specifically targeting the property copying in emutls_decl().
 */

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

/* For Windows DLL import/export attributes */
#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_EXPORT
#define DLL_IMPORT
#endif

/* Prevent optimization of TLS variable usage */
volatile void* volatile_global_ptr;
volatile int volatile_global_int;

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;  /* No initializer = tentative definition */

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
/* We'll define this in a separate compilation unit or use forward declaration */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
DLL_IMPORT __thread int tls_imported;
#else
/* On non-Windows, we'll create a similar variable without dllimport */
__thread int tls_imported __attribute__((weak)) = 5;
#endif

/* 6. Preserved TLS variable - may trigger DECL_PRESERVE_P */
/* Use 'used' attribute to suggest preservation */
__thread int tls_preserved __attribute__((used)) = 6;

/* 7. Public TLS variable with noinit - triggers TREE_PUBLIC */
__thread int tls_public = 7;

/* 8. Static TLS (non-public) for context variation */
static __thread int tls_static = 8;

/* 9. TLS in different storage duration context */
__thread int* tls_pointer = 0;

/* ===== HELPER FUNCTIONS ===== */

/* Force noinline to ensure function isn't optimized away */
__attribute__((noinline, used))
static void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    volatile_global_ptr = (void*)&tls_weak;
    volatile_global_ptr = (void*)&tls_hidden;
    volatile_global_ptr = (void*)&tls_common;
    volatile_global_ptr = (void*)&tls_external;
    volatile_global_ptr = (void*)&tls_imported;
    volatile_global_ptr = (void*)&tls_preserved;
    volatile_global_ptr = (void*)&tls_public;
    volatile_global_ptr = (void*)&tls_static;
    volatile_global_ptr = (void*)&tls_pointer;
    
    /* Use values to prevent dead store elimination */
    tls_common = tls_weak + tls_hidden;
    tls_pointer = &tls_common;
    
    volatile_global_int = tls_weak;
    volatile_global_int = tls_hidden;
    volatile_global_int = tls_common;
    volatile_global_int = tls_preserved;
    volatile_global_int = tls_public;
    volatile_global_int = tls_static;
    
    /* Force computation with TLS variables */
    int checksum = tls_weak + tls_hidden * 2 + tls_common * 3 +
                   tls_preserved * 4 + tls_public * 5 + tls_static * 6;
    
    volatile_global_int = checksum;
}

/* Another function to ensure TLS variables are used in multiple contexts */
__attribute__((noinline))
static int compute_with_tls(void) {
    /* Create a volatile barrier */
    asm volatile("" : : : "memory");
    
    /* Use TLS variables in different ways */
    tls_weak++;
    tls_hidden = tls_weak * 2;
    tls_common = tls_hidden + tls_weak;
    
    return tls_weak + tls_hidden + tls_common;
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    /* Initialize tls_common if not already initialized elsewhere */
    tls_common = 3;
    
    /* Initialize tls_pointer */
    tls_pointer = &tls_common;
    
    /* Use TLS variables in helper function */
    use_tls_variables();
    
    /* Use TLS variables in computation */
    int result = compute_with_tls();
    
    /* Force comparison of TLS addresses (prevents optimization) */
    if (&tls_weak != &tls_hidden) {
        volatile_global_int = 1;
    }
    
    /* Create a checksum that depends on all TLS variables */
    int final_checksum = 
        tls_weak + tls_hidden + tls_common + 
        tls_preserved + tls_public + tls_static;
    
    /* Print something to ensure execution */
    printf("TLS test: result=%d, checksum=%d\n", result, final_checksum);
    
    /* Additional forced usage */
    volatile int* volatile_ptr;
    volatile_ptr = &tls_weak;
    volatile_ptr = &tls_hidden;
    volatile_ptr = &tls_common;
    
    return final_checksum > 0 ? 0 : 1;
}

/* ===== EXTERNAL TLS DEFINITION ===== */
/* This would normally be in a separate file, but we include it here
 * with a guard to simulate multi-file compilation when using -fcommon */
#ifndef NO_EXTERNAL_DEF
__thread int tls_external = 42;
#endif

/* ===== WINDOWS-SPECIFIC DLL EXPORT ===== */
#ifdef _WIN32
/* In a real DLL, this would be exported */
DLL_EXPORT __thread int tls_imported = 99;
#endif
