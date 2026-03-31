/* Main test file for emulated TLS coverage */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode */
#if defined(__GNUC__) && !defined(__HAVE_TLS)
#warning "Using emulated TLS mode"
#endif

/* Global volatile array to prevent optimization */
volatile uintptr_t tls_addresses[10] = {0};

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, use weak import simulation */
extern __thread int tls_imported __attribute__((weak));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. TLS with initializer (not BSS) */
__thread int tls_init = 42;

/* 8. Public TLS with specific section */
__thread int tls_public __attribute__((section(".tls_data")));

/* 9. Static TLS (non-public context) */
static __thread int tls_static = 100;

/* 10. TLS in function scope (different DECL_CONTEXT) */
void function_with_tls(void) {
    static __thread int tls_function_scope = 200;
    tls_addresses[9] = (uintptr_t)&tls_function_scope;
}

/* Helper function marked noinline to prevent optimization */
__attribute__((noinline, used))
void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = (uintptr_t)&tls_weak;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    tls_addresses[2] = (uintptr_t)&tls_common;
    tls_addresses[3] = (uintptr_t)&tls_external;
    tls_addresses[4] = (uintptr_t)&tls_imported;
    tls_addresses[5] = (uintptr_t)&tls_preserved;
    tls_addresses[6] = (uintptr_t)&tls_init;
    tls_addresses[7] = (uintptr_t)&tls_public;
    tls_addresses[8] = (uintptr_t)&tls_static;
    
    /* Use the values to prevent dead code elimination */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = 5;
    tls_public = 7;
    tls_static = 9;
    
    /* Perform computation using TLS values */
    int sum = tls_weak + tls_hidden + tls_common + tls_preserved + 
              tls_public + tls_static + tls_init;
    
    /* Store result in TLS to create dependencies */
    tls_common = sum;
    
    /* Call function with scope-local TLS */
    function_with_tls();
}

/* Another function that uses TLS to ensure proper initialization */
__attribute__((noinline))
uintptr_t compute_tls_checksum(void) {
    uintptr_t checksum = 0;
    
    /* XOR all TLS addresses */
    for (int i = 0; i < 10; i++) {
        checksum ^= tls_addresses[i];
    }
    
    /* Add values from TLS variables */
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_common;
    checksum += tls_preserved;
    checksum += tls_public;
    checksum += tls_static;
    checksum += tls_init;
    
    return checksum;
}

int main(void) {
    /* Use TLS variables */
    use_tls_variables();
    
    /* Compute and print checksum to ensure TLS is used */
    uintptr_t checksum = compute_tls_checksum();
    
    /* Force comparison of TLS addresses */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print something to prevent optimization */
    printf("TLS checksum: %lu\n", (unsigned long)checksum);
    printf("tls_init value: %d\n", tls_init);
    printf("tls_common value: %d\n", tls_common);
    
    return 0;
}
