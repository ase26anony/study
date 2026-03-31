/* test_emutls_coverage.c
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 -o test test_emutls_coverage.c
 * For Windows: gcc -O2 -ftls-model=emulated -D_WIN32 -o test test_emutls_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by checking if native TLS is not available */
#ifndef __HAVE_TLS
#define USE_EMULATED_TLS
#endif

/* Global volatile array to prevent optimization */
volatile uintptr_t tls_addresses[10];
volatile int tls_values[10];
int checksum = 0;

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers TREE_PUBLIC and DECL_EXTERNAL */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows only) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, just declare as regular TLS */
__thread int tls_imported;
#endif

/* 6. Preserved TLS variable with multiple attributes */
__thread int tls_preserved __attribute__((used));

/* 7. TLS with initializer (non-BSS) */
__thread int tls_init = 42;

/* 8. TLS in function scope for DECL_CONTEXT variation */
static void func_with_tls(void) {
    static __thread int tls_in_func = 100;
    tls_addresses[7] = (uintptr_t)&tls_in_func;
    tls_values[7] = tls_in_func++;
}

/* 9. TLS with noinline attribute on containing function */
__attribute__((noinline)) static void use_tls_preserved(void) {
    tls_preserved = 0xABCD;
    tls_addresses[5] = (uintptr_t)&tls_preserved;
    tls_values[5] = tls_preserved;
}

/* Helper function that uses all TLS variables to prevent dead code elimination */
__attribute__((noinline)) static void use_all_tls_variables(void) {
    /* Take addresses and use values of all TLS variables */
    
    /* tls_weak */
    tls_weak = 1;
    tls_addresses[0] = (uintptr_t)&tls_weak;
    tls_values[0] = tls_weak;
    
    /* tls_hidden */
    tls_hidden = 2;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    tls_values[1] = tls_hidden;
    
    /* tls_common */
    tls_common = 3;
    tls_addresses[2] = (uintptr_t)&tls_common;
    tls_values[2] = tls_common;
    
    /* tls_external - declared extern, will be defined elsewhere or left unresolved */
    tls_addresses[3] = (uintptr_t)&tls_external;
    tls_values[3] = 4; /* Use a dummy value */
    
    /* tls_imported */
    tls_imported = 5;
    tls_addresses[4] = (uintptr_t)&tls_imported;
    tls_values[4] = tls_imported;
    
    /* tls_preserved */
    use_tls_preserved();
    
    /* tls_init */
    tls_init++;
    tls_addresses[6] = (uintptr_t)&tls_init;
    tls_values[6] = tls_init;
    
    /* tls_in_func */
    func_with_tls();
    
    /* Force compiler to consider all addresses different */
    if (&tls_weak != &tls_hidden) {
        checksum++;
    }
    if (&tls_common != &tls_init) {
        checksum++;
    }
}

/* Define the external TLS variable to satisfy the linker */
#ifdef DEFINE_EXTERNAL_TLS
__thread int tls_external = 99;
#endif

int main(void) {
    int i;
    
    /* Use all TLS variables */
    use_all_tls_variables();
    
    /* Calculate a checksum from TLS values to ensure they're used */
    int local_checksum = 0;
    for (i = 0; i < 8; i++) {
        local_checksum += tls_values[i];
    }
    
    /* Print something to prevent optimization and verify execution */
    printf("TLS checksum: %d\n", local_checksum);
    printf("Address checksum: %d\n", checksum);
    
    /* Print TLS addresses to force their use */
    for (i = 0; i < 8; i++) {
        printf("TLS %d address: %p\n", i, (void*)tls_addresses[i]);
    }
    
    return 0;
}
