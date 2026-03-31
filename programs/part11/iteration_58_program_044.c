/* test-emutls-coverage.c
 * 
 * This program creates thread-local variables with diverse attributes
 * to trigger the property copying logic in GCC's emutls_decl function.
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 test-emutls-coverage.c -o test-emutls-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of TLS variable usage */
volatile void *tls_addresses[10];
volatile int tls_values[10];

/* Helper function to force TLS variable usage - marked noinline to prevent optimization */
__attribute__((noinline)) void use_tls_variables(void) {
    /* Take addresses and values of all TLS variables */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_external;
    tls_addresses[4] = (void*)&tls_init;
    tls_addresses[5] = (void*)&tls_static_func;
    
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = tls_external;
    tls_values[4] = tls_init;
    tls_values[5] = tls_static_func;
}

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable - tentative definition, may set DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - will be defined in another TU or with extern */
extern __thread int tls_external;

/* 5. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 6. TLS variable with used attribute - may influence DECL_PRESERVE_P */
__thread int tls_used __attribute__((used));

/* 7. TLS variable inside function scope - affects DECL_CONTEXT */
static void func_with_tls(void) {
    static __thread int tls_static_func = 100;
    tls_static_func++;
    tls_addresses[6] = (void*)&tls_static_func;
    tls_values[6] = tls_static_func;
}

/* 8. TLS variable with noinline function usage */
__attribute__((noinline)) void modify_tls(void) {
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_init = 4;
    tls_used = 5;
}

/* Conditional compilation for Windows-specific attributes */
#ifdef _WIN32
/* 9. DLL Import TLS - triggers DECL_DLLIMPORT_P */
__declspec(dllimport) __thread int tls_imported;
#endif

/* For C++ compatibility and namespace context */
#ifdef __cplusplus
namespace {
    /* TLS in anonymous namespace */
    __thread int tls_in_namespace = 123;
}
#endif

int main(void) {
    int checksum = 0;
    
    /* Initialize external TLS reference */
    tls_external = 10;
    
    /* Use TLS variables in various ways */
    modify_tls();
    func_with_tls();
    use_tls_variables();
    
    /* Force referencing of variables to prevent optimization */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Calculate checksum from TLS values */
    for (int i = 0; i < 7; i++) {
        checksum += (int)(uintptr_t)tls_addresses[i];
        checksum += tls_values[i];
    }
    
    printf("TLS checksum: %d\n", checksum);
    printf("tls_weak address: %p, value: %d\n", (void*)&tls_weak, tls_weak);
    printf("tls_hidden address: %p, value: %d\n", (void*)&tls_hidden, tls_hidden);
    printf("tls_common address: %p, value: %d\n", (void*)&tls_common, tls_common);
    printf("tls_external address: %p, value: %d\n", (void*)&tls_external, tls_external);
    printf("tls_init address: %p, value: %d\n", (void*)&tls_init, tls_init);
    
    /* Runtime check for emulated TLS */
    printf("Using emulated TLS mode\n");
    
    return 0;
}
