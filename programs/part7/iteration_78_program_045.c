/* tls_main.c - Main file with various TLS variable declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation flags */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O0")
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* Public TLS with explicit visibility and used attribute */
__thread int tls_public __attribute__((used, visibility("default"))) = 42;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 100;

/* Common linkage TLS (tentative definition) */
__thread int tls_common;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* DLL import simulation (even on non-Windows) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate DLL import with a special attribute */
__thread int tls_imported __attribute__((dllimport));
#endif

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* TLS with thread-specific context */
static __thread void* tls_context;

/* ========== FUNCTION DECLARATIONS ========== */
void test_tls_access(void);
void test_tls_modification(void);
uint32_t compute_tls_checksum(void);

/* Function with static TLS variable */
static void function_with_static_tls(void) {
    /* Static TLS inside function - different DECL_CONTEXT */
    static __thread int tls_in_function = 999;
    tls_in_function++;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&tls_in_function));
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initialize common TLS */
    tls_common = 50;
    
    /* Access all TLS variables to ensure they're instantiated */
    tls_public += 1;
    tls_weak += 2;
    tls_hidden += 3;
    
    /* Try to access imported TLS (may be zero if not defined) */
    int import_val = 0;
#ifdef _WIN32
    import_val = tls_imported;
#else
    /* Use address to force reference without actual access */
    asm volatile("" : : "r"(&tls_imported));
#endif
    
    /* Access external TLS */
    tls_external = 123;
    
    /* Set thread-specific context */
    tls_context = (void*)&tls_public;
    
    /* Call functions that use TLS */
    test_tls_access();
    function_with_static_tls();
    test_tls_modification();
    
    /* Compute and print checksum */
    uint32_t checksum = compute_tls_checksum();
    printf("TLS checksum: 0x%08x\n", checksum);
    
    /* Verify all attributes were copied */
    printf("tls_public address: %p\n", (void*)&tls_public);
    printf("tls_weak address: %p\n", (void*)&tls_weak);
    printf("tls_common address: %p\n", (void*)&tls_common);
    printf("tls_hidden address: %p\n", (void*)&tls_hidden);
    
    return 0;
}

/* Force usage of all TLS variables in a separate function */
uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    sum += tls_external;
    
    /* Access weak external if available */
    if (&tls_external_weak != NULL) {
        sum += tls_external_weak;
    }
    
    /* Use context pointer */
    if (tls_context != NULL) {
        sum += (uintptr_t)tls_context & 0xFFFF;
    }
    
    return sum;
}
