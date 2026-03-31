/* tls_emutest.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && !defined(__HAVE_TLS)
#pragma GCC tls_model emulated
#endif

/* For Windows DLL import/export attributes */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((visibility("default")))
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* Prevent optimization of TLS variable usage */
#define USE_TLS_VAR(var) do { \
    volatile void *volatile ptr = &(var); \
    (void)ptr; \
} while(0)

/* Force function to not be inlined */
#define NOINLINE __attribute__((noinline))

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));
__thread int tls_default_vis __attribute__((visibility("default")));

/* 3. Common TLS variable (tentative definition) - may set DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
DLL_IMPORT __thread int tls_imported;
#else
/* On non-Windows, use visibility as proxy */
__thread int tls_imported __attribute__((visibility("default")));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
/* Use 'used' attribute and ensure it's referenced in preserved function */
__thread int tls_preserved __attribute__((used));

/* 7. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 8. TLS with multiple combined attributes */
__thread int tls_combined __attribute__((weak, visibility("hidden")));

/* 9. Static TLS inside function - different DECL_CONTEXT */
static void function_with_tls(void) {
    static __thread int tls_in_function = 100;
    USE_TLS_VAR(tls_in_function);
}

/* ========== HELPER FUNCTIONS ========== */

/* Global array to store TLS addresses and prevent optimization */
volatile void *tls_addresses[10];

NOINLINE static void process_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = &tls_weak;
    tls_addresses[1] = &tls_hidden;
    tls_addresses[2] = &tls_common;
    tls_addresses[3] = &tls_external;
    tls_addresses[4] = &tls_imported;
    tls_addresses[5] = &tls_preserved;
    tls_addresses[6] = &tls_init;
    tls_addresses[7] = &tls_combined;
    
    /* Use the TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = 4;
    tls_init = tls_init * 2; /* 42 * 2 = 84 */
    tls_combined = tls_weak + tls_hidden; /* 1 + 2 = 3 */
    
    /* Force computation that depends on TLS values */
    volatile int checksum = 0;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_common;
    checksum += tls_preserved;
    checksum += tls_init;
    checksum += tls_combined;
    
    tls_addresses[8] = (void *)(intptr_t)checksum;
    
    /* Call function with internal TLS */
    function_with_tls();
}

/* Another function that uses TLS to ensure they're preserved */
NOINLINE static void another_tls_user(void) {
    /* Different usage pattern */
    tls_weak++;
    tls_hidden--;
    tls_common = tls_weak * tls_hidden;
    
    /* Store result to prevent optimization */
    volatile int result = tls_common;
    tls_addresses[9] = (void *)(intptr_t)result;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize external TLS reference */
    tls_external = 99;
    
    /* Process TLS variables in helper function */
    process_tls_variables();
    
    /* Use TLS in another function */
    another_tls_user();
    
    /* Force comparison of TLS addresses */
    if (&tls_weak != &tls_hidden) {
        /* This ensures both addresses are taken */
        volatile int diff = 1;
        (void)diff;
    }
    
    /* Create observable output based on TLS values */
    int final_sum = tls_weak + tls_hidden + tls_common + 
                   tls_preserved + tls_init + tls_combined;
    
    /* Use the sum to prevent dead code elimination */
    volatile int output = final_sum;
    
    /* Return value based on TLS computation */
    return (output > 0) ? 0 : 1;
}

/* ========== SECOND TRANSLATION UNIT (if split compilation) ========== */
/* 
   For full testing, compile this separately and link:
   
   tls_emutest_ext.c:
   ------------------
   __thread int tls_external = 99;
   
   #ifdef _WIN32
   DLL_EXPORT __thread int tls_imported = 77;
   #endif
   
   Compile with: gcc -c tls_emutest_ext.c -O2 -ftls-model=emulated
   Link with: gcc tls_emutest.c tls_emutest_ext.o -O2 -ftls-model=emulated -pthread
*/
