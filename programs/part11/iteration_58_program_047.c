/* tls_emutest.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode */
#ifndef __HAVE_TLS
#define __HAVE_TLS 0
#endif

#if __HAVE_TLS
#warning "Native TLS support detected - may not trigger emulated TLS paths"
#endif

/* Windows DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#endif

/* Prevent optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))
#define PRESERVE __attribute__((used, noinline))

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;

/* Common TLS variable - tentative definition, may trigger DECL_COMMON */
__thread int tls_common;

/* External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* DLL Import TLS (for Windows targets) - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
DLL_IMPORT __thread int tls_imported;
#else
/* Simulate similar behavior on non-Windows */
extern __thread int tls_imported;
#endif

/* Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved PRESERVE = 5;

/* TLS with public linkage */
__thread int tls_public __attribute__((used)) = 6;

/* TLS in different contexts to affect DECL_CONTEXT */
static void inner_function(void) {
    /* TLS with function scope context */
    static __thread int tls_function_scope = 7;
    volatile int *p = &tls_function_scope;
    (void)p;
}

/* ========== HELPER FUNCTIONS ========== */

/* Global array to store results and prevent optimization */
volatile int results[8] = {0};

NOINLINE static void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    volatile int *addr_weak = &tls_weak;
    volatile int *addr_hidden = &tls_hidden;
    volatile int *addr_common = &tls_common;
    volatile int *addr_external = &tls_external;
    volatile int *addr_imported = &tls_imported;
    volatile int *addr_preserved = &tls_preserved;
    volatile int *addr_public = &tls_public;
    
    /* Store addresses to prevent optimization */
    results[0] = (int)(long)addr_weak;
    results[1] = (int)(long)addr_hidden;
    results[2] = (int)(long)addr_common;
    results[3] = (int)(long)addr_external;
    results[4] = (int)(long)addr_imported;
    results[5] = (int)(long)addr_preserved;
    results[6] = (int)(long)addr_public;
    
    /* Use the values */
    tls_common = tls_weak + tls_hidden;
    results[7] = tls_common;
    
    /* Force different addresses check */
    if (addr_weak != addr_hidden) {
        results[0] |= 0x1;
    }
}

/* Function that preserves TLS variables */
PRESERVE static void preserve_tls_access(void) {
    tls_preserved++;
    tls_public += tls_preserved;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 3;
    
    /* Call function that uses TLS variables */
    use_tls_variables();
    
    /* Call preserving function */
    preserve_tls_access();
    
    /* Use inner function with scoped TLS */
    inner_function();
    
    /* Calculate checksum from results */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum ^= results[i];
    }
    
    /* Print something to ensure execution */
    printf("TLS test checksum: %d\n", checksum);
    printf("tls_weak address: %p\n", (void*)&tls_weak);
    printf("tls_hidden address: %p\n", (void*)&tls_hidden);
    
    return checksum & 0xFF;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This would normally be in a separate file, but we include it here
   for simplicity in a single-file test */
__thread int tls_external = 4;

#ifdef _WIN32
__thread int tls_imported = 8;
#else
__thread int tls_imported = 8;
#endif
