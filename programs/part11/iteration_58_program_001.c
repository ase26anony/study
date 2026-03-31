/* Main test file for emulated TLS attribute copying coverage */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode */
#ifndef __HAVE_TLS
#define __HAVE_TLS 0
#endif

#if __HAVE_TLS
#warning "Native TLS support detected - may not trigger emulated TLS paths"
#endif

/* Prevent optimization */
volatile void *volatile_ptr;
volatile int volatile_result;

/* ===== TLS VARIABLES WITH VARIOUS ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. Preserved TLS variable - may trigger DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 6. Public TLS with initialization */
__thread int tls_public = 42;

/* 7. Static TLS (non-public context) - affects DECL_CONTEXT */
static __thread int tls_static = 100;

/* 8. TLS used in multiple functions */
__thread int tls_multi = 1;

/* Windows-specific DLL import (guarded) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar attribute on non-Windows */
__thread int tls_imported __attribute__((weak));
#endif

/* ===== HELPER FUNCTIONS ===== */

/* Noinline function to ensure TLS variables are processed */
__attribute__((noinline, used)) 
static uintptr_t compute_tls_checksum(void) {
    uintptr_t sum = 0;
    
    /* Take addresses of all TLS variables */
    sum += (uintptr_t)&tls_weak;
    sum += (uintptr_t)&tls_hidden;
    sum += (uintptr_t)&tls_common;
    sum += (uintptr_t)&tls_external;
    sum += (uintptr_t)&tls_preserved;
    sum += (uintptr_t)&tls_public;
    sum += (uintptr_t)&tls_static;
    sum += (uintptr_t)&tls_multi;
    sum += (uintptr_t)&tls_imported;
    
    /* Use their values */
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_external;
    sum += tls_preserved;
    sum += tls_public;
    sum += tls_static;
    sum += tls_multi;
    sum += tls_imported;
    
    /* Modify some values */
    tls_multi = sum & 0xFF;
    tls_common = tls_multi * 2;
    
    return sum;
}

/* Another function using TLS to ensure proper context */
__attribute__((noinline))
static void modify_tls_values(void) {
    tls_hidden = 0xABCD;
    tls_preserved = 0x1234;
    
    /* Force compiler to keep these variables */
    volatile_ptr = &tls_hidden;
    volatile_ptr = &tls_preserved;
}

/* Function with local static TLS */
__attribute__((noinline))
static void function_with_local_tls(void) {
    static __thread int local_tls = 999;
    local_tls++;
    volatile_result = local_tls;
}

/* ===== MAIN FUNCTION ===== */
int main(void) {
    uintptr_t checksum;
    
    /* Initialize some TLS variables */
    tls_weak = 10;
    tls_hidden = 20;
    tls_common = 30;
    tls_preserved = 40;
    
    /* Call functions that use TLS */
    modify_tls_values();
    function_with_local_tls();
    
    /* Compute checksum using all TLS variables */
    checksum = compute_tls_checksum();
    
    /* Force compiler to keep all TLS variables alive */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Use the checksum to prevent optimization */
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    printf("tls_external value: %d\n", tls_external);
    printf("tls_public value: %d\n", tls_public);
    printf("tls_static value: %d\n", tls_static);
    printf("tls_multi value: %d\n", tls_multi);
    
    /* Additional forced usage */
    volatile_ptr = &tls_weak;
    volatile_ptr = &tls_hidden;
    volatile_ptr = &tls_common;
    volatile_ptr = &tls_external;
    
    return (int)(checksum & 0xFF);
}
