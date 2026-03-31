/* tls_emutest.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode */
#ifndef __HAVE_TLS
#define __HAVE_TLS 0
#endif

#if __HAVE_TLS
#warning "Native TLS support detected - may not trigger emulated TLS paths"
#endif

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of TLS variable usage */
volatile void *tls_addresses[10];
volatile int tls_values[10];
int global_checksum = 0;

/* Helper function to ensure TLS variables are used */
__attribute__((noinline, used))
static void use_tls_variables(void) {
    int sum = 0;
    
    /* 1. Weak TLS variable - triggers DECL_WEAK copying */
    extern __thread int tls_weak __attribute__((weak));
    tls_addresses[0] = (void*)&tls_weak;
    if (&tls_weak != NULL) {
        tls_weak = 100;
        sum += tls_weak;
    }
    
    /* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
    __thread int tls_hidden __attribute__((visibility("hidden"))) = 200;
    tls_addresses[1] = (void*)&tls_hidden;
    sum += tls_hidden;
    
    /* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
    __thread int tls_common;  /* No initializer */
    tls_addresses[2] = (void*)&tls_common;
    tls_common = 300;
    sum += tls_common;
    
    /* 4. External TLS declaration - triggers TREE_PUBLIC and DECL_EXTERNAL */
    extern __thread int tls_external;
    tls_addresses[3] = (void*)&tls_external;
    sum += tls_external;
    
    /* 5. Public TLS with used attribute - may influence DECL_PRESERVE_P */
    __thread int tls_preserved __attribute__((used)) = 500;
    tls_addresses[4] = (void*)&tls_preserved;
    sum += tls_preserved;
    
    /* 6. TLS with noinline function context - affects DECL_CONTEXT */
    {
        static __thread int tls_in_function = 600;
        tls_addresses[5] = (void*)&tls_in_function;
        sum += tls_in_function;
    }
    
    /* 7. TLS with specific section */
    __thread int tls_sectioned __attribute__((section(".tls_data"))) = 700;
    tls_addresses[6] = (void*)&tls_sectioned;
    sum += tls_sectioned;
    
    /* Store the sum to prevent optimization */
    tls_values[0] = sum;
    global_checksum += sum;
}

/* Windows-specific DLL import test */
#ifdef _WIN32
/* Simulate DLL import scenario */
#ifdef BUILDING_DLL
__declspec(dllexport) __thread int tls_exported = 800;
#else
__declspec(dllimport) __thread int tls_imported;
#endif
#endif

/* External TLS definition (for the external declaration above) */
__thread int tls_external = 400;

/* Weak TLS definition (provides a definition if none in other modules) */
__thread int tls_weak = 150;

/* Another TLS in different linkage context */
static __thread int tls_static = 900;

/* Function that uses static TLS to ensure it's preserved */
__attribute__((noinline, used))
static void use_static_tls(void) {
    tls_static++;
    tls_addresses[7] = (void*)&tls_static;
    tls_values[1] = tls_static;
    global_checksum += tls_static;
}

/* Main function with observable TLS usage */
int main(void) {
    printf("Testing emulated TLS coverage\n");
    
    /* Force all TLS variables to be referenced */
    use_tls_variables();
    use_static_tls();
    
    /* Additional direct usage to prevent optimization */
    volatile int *volatile ptr;
    
    ptr = &tls_hidden;
    *ptr += 1;
    
    ptr = &tls_common;
    *ptr += 2;
    
    ptr = &tls_preserved;
    *ptr += 3;
    
    /* Create a checksum from TLS addresses to ensure they're unique */
    uintptr_t addr_sum = 0;
    for (int i = 0; i < 8; i++) {
        if (tls_addresses[i]) {
            addr_sum += (uintptr_t)tls_addresses[i];
        }
    }
    
    /* Use the checksum in output */
    printf("TLS address checksum: 0x%lx\n", (unsigned long)addr_sum);
    printf("TLS value checksum: %d\n", global_checksum);
    
    /* Force comparison of TLS addresses */
    if (&tls_hidden != &tls_common) {
        printf("TLS addresses differ as expected\n");
    }
    
    return 0;
}
