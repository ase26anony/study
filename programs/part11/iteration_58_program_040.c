/* test_emutls.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode */
#ifndef __HAVE_TLS
#define __HAVE_TLS 0
#endif

#if __HAVE_TLS
#error "This test requires emulated TLS, not native TLS"
#endif

/* Prevent optimizations from removing unused TLS variables */
#define USED __attribute__((used))
#define NOINLINE __attribute__((noinline))
#define PRESERVED __attribute__((used, retain))

/* For Windows DLL import/export simulation */
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORT
#define DLLIMPORT
#endif

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));
__thread int tls_default __attribute__((visibility("default")));
__thread int tls_internal __attribute__((visibility("internal")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
DLLIMPORT __thread int tls_imported;
#endif

/* 6. Preserved TLS variable - influences DECL_PRESERVE_P */
__thread int tls_preserved PRESERVED;

/* 7. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 8. TLS in different contexts - affects DECL_CONTEXT */
static void function_context(void) {
    /* TLS with function scope context */
    static __thread int tls_function_scope = 100;
    volatile int *p = &tls_function_scope;
    (void)p;
}

/* 9. Public TLS with used attribute - ensures TREE_USED is set */
__thread int tls_public_used USED = 1;

/* ===== HELPER FUNCTIONS ===== */

/* Global volatile array to prevent optimization */
volatile int results[10];

/* NOINLINE function that uses all TLS variables */
NOINLINE static void use_all_tls_variables(void) {
    /* Take addresses of all TLS variables */
    volatile int *ptrs[] = {
        &tls_weak,
        &tls_hidden,
        &tls_default,
        &tls_internal,
        &tls_common,
        &tls_preserved,
        &tls_init,
        &tls_public_used,
        NULL
    };
    
    /* Perform computations with TLS variables */
    int checksum = 0;
    
    tls_weak = 1;
    tls_hidden = 2;
    tls_default = 3;
    tls_internal = 4;
    tls_common = 5;
    tls_preserved = 6;
    tls_init = 7;
    tls_public_used = 8;
    
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_default;
    checksum += tls_internal;
    checksum += tls_common;
    checksum += tls_preserved;
    checksum += tls_init;
    checksum += tls_public_used;
    
    /* Store results in volatile array */
    results[0] = checksum;
    
    /* Force different addresses */
    if (&tls_weak != &tls_hidden) {
        results[1] = 1;
    }
    
    /* Call function with function-scope TLS */
    function_context();
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    /* Initialize external TLS if available */
    extern int tls_external;
    tls_external = 99;
    
#ifdef _WIN32
    /* Initialize imported TLS if available */
    tls_imported = 77;
#endif
    
    /* Use all TLS variables through helper */
    use_all_tls_variables();
    
    /* Print checksum to ensure TLS variables are used */
    printf("TLS checksum: %d\n", results[0]);
    
    /* Additional checks to force TLS variable references */
    printf("TLS addresses:\n");
    printf("  tls_weak: %p\n", (void*)&tls_weak);
    printf("  tls_hidden: %p\n", (void*)&tls_hidden);
    printf("  tls_common: %p\n", (void*)&tls_common);
    printf("  tls_init: %p\n", (void*)&tls_init);
    
    return 0;
}

/* ===== SECOND TRANSLATION UNIT (if split compilation) ===== */
/* 
   For full coverage, compile this separately and link:
   
   File2.c:
   __thread int tls_external = 42;
   
   #ifdef _WIN32
   __declspec(dllexport) __thread int tls_imported = 123;
   #endif
*/
