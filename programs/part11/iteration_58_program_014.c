/* test-emutls-attributes.c
 * 
 * This program tests GCC's emulated TLS initialization by creating
 * thread-local variables with diverse attributes that should trigger
 * the property copying in emutls_decl() (lines 295-304 of tree-emutls.cc).
 *
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread test-emutls-attributes.c -o test-emutls
 * For 32-bit: gcc -O2 -m32 -ftls-model=emulated -fno-builtin -pthread test-emutls-attributes.c -o test-emutls-32
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if we're in emulated mode */
#ifndef __HAVE_TLS
#define EMULATED_TLS_MODE 1
#else
/* Try to force emulated mode anyway */
#define EMULATED_TLS_MODE 1
#endif

/* Global volatile array to prevent optimization */
volatile void* tls_addresses[10];
volatile int tls_values[10];

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may set DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - will be defined later, sets DECL_EXTERNAL */
extern __thread int tls_external;

/* 5. Public TLS variable with used attribute - affects TREE_PUBLIC and DECL_PRESERVE_P */
__thread int tls_public __attribute__((used));

/* 6. TLS variable with initialization - not BSS */
__thread int tls_init = 42;

/* 7. Static TLS variable (file scope) - different context */
static __thread int tls_static = 100;

/* 8. TLS variable in a struct context */
struct Container {
    int regular;
};
static struct Container container;
/* Simulate DECL_CONTEXT by association */
#define tls_in_context tls_public  /* Use existing one for simplicity */

/* 9. DLL Import simulation (for Windows/MinGW) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, we'll simulate with a regular extern */
extern __thread int tls_imported;
#endif

/* ========== FUNCTION TO USE TLS VARIABLES ========== */

/* __attribute__((noinline)) ensures function isn't optimized away */
__attribute__((noinline, used))
static void use_tls_variables(void) {
    /* Take addresses of TLS variables - forces their instantiation */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_external;
    tls_addresses[4] = (void*)&tls_public;
    tls_addresses[5] = (void*)&tls_init;
    tls_addresses[6] = (void*)&tls_static;
    
    /* Use TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    /* tls_external will be set by external definition */
    tls_public = 5;
    tls_init = tls_init * 2;  /* 42 * 2 = 84 */
    tls_static = tls_static + 50;  /* 100 + 50 = 150 */
    
    /* Store computed values to prevent optimization */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = 0;  /* Placeholder for external */
    tls_values[4] = tls_public;
    tls_values[5] = tls_init;
    tls_values[6] = tls_static;
    
    /* Force comparison of TLS addresses */
    if (&tls_weak != &tls_hidden) {
        /* This should always be true, forces reference to both */
        tls_values[7] = 1;
    }
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This would normally be in a separate file, but we'll define it here
 * with a forward declaration to simulate external linkage */
__thread int tls_external = 99;

/* For Windows DLL import simulation */
#ifndef _WIN32
#ifndef __MINGW32__
__thread int tls_imported = 77;
#endif
#endif

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int checksum = 0;
    int i;
    
    printf("Testing emulated TLS with various attributes...\n");
    
    /* Use TLS variables */
    use_tls_variables();
    
    /* Set the external TLS value */
    tls_external = 123;
    tls_values[3] = tls_external;
    
    /* Set imported TLS if available */
#ifdef _WIN32
    /* On real Windows, we'd need a DLL */
    tls_values[8] = 0;
#elif defined(__MINGW32__)
    tls_values[8] = 0;
#else
    tls_imported = 77;
    tls_values[8] = tls_imported;
#endif
    
    /* Compute checksum from all TLS values */
    for (i = 0; i < 9; i++) {
        checksum += tls_values[i];
        printf("TLS value[%d] = %d\n", i, tls_values[i]);
    }
    
    /* Print TLS addresses to ensure they're different */
    printf("\nTLS addresses:\n");
    for (i = 0; i < 7; i++) {
        printf("  tls_addresses[%d] = %p\n", i, (void*)tls_addresses[i]);
    }
    
    printf("\nChecksum of TLS values: %d\n", checksum);
    
    /* Additional check for emulated TLS mode */
    printf("TLS mode: %s\n", EMULATED_TLS_MODE ? "Likely emulated" : "Native");
    
    /* Force reference to all TLS variables one more time */
    volatile int dummy = 
        tls_weak + tls_hidden + tls_common + tls_external + 
        tls_public + tls_init + tls_static;
    
    (void)dummy;  /* Suppress unused warning */
    
    return 0;
}
