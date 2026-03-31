/* test-emutls-attributes.c
 * 
 * This program creates thread-local variables with various attributes
 * to trigger GCC's emulated TLS initialization logic, specifically
 * targeting the property copying in emutls_decl() function.
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode - this is the key to hitting the target code */
/* Compile with: -O2 -ftls-model=emulated -fno-builtin */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers TREE_PUBLIC and DECL_EXTERNAL */
extern __thread int tls_external;

/* 5. DLL Import TLS simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar behavior on non-Windows */
__thread int tls_imported __attribute__((weak));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. Public TLS with initializer - ensures it's not optimized away */
__thread int tls_initialized = 42;

/* 8. Static TLS inside a function - gives non-global DECL_CONTEXT */
static void function_with_tls(void) {
    static __thread int tls_in_function = 100;
    tls_in_function++;
}

/* 9. TLS with section attribute - additional complexity */
__thread int tls_in_section __attribute__((section(".tls_data"))) = 99;

/* Global volatile array to prevent optimization */
volatile uintptr_t tls_addresses[10];
volatile int tls_values[10];

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline, used))
static void use_all_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = (uintptr_t)&tls_weak;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    tls_addresses[2] = (uintptr_t)&tls_common;
    tls_addresses[3] = (uintptr_t)&tls_external;
    tls_addresses[4] = (uintptr_t)&tls_imported;
    tls_addresses[5] = (uintptr_t)&tls_preserved;
    tls_addresses[6] = (uintptr_t)&tls_initialized;
    
    /* Use the TLS variables in computations */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    /* tls_external is defined elsewhere */
    tls_imported = 5;
    tls_preserved = 6;
    
    /* Compute checksum using TLS values */
    int checksum = tls_weak + tls_hidden + tls_common + tls_imported + 
                   tls_preserved + tls_initialized;
    
    tls_values[0] = checksum;
    
    /* Use function-scoped TLS */
    function_with_tls();
    
    /* Use section TLS */
    tls_in_section = checksum % 100;
    tls_values[1] = tls_in_section;
    
    /* Force different addresses check */
    if (&tls_weak != &tls_hidden) {
        tls_values[2] = 1;
    }
}

/* Another translation unit would define tls_external */
/* In a separate file: __thread int tls_external = 123; */

/* For multi-TU testing, we define it here but mark as weak 
   to simulate external definition */
__thread int tls_external __attribute__((weak));

int main(void) {
    /* Initialize external TLS if not defined elsewhere */
    if (&tls_external != NULL) {
        tls_external = 123;
    }
    
    /* Use all TLS variables */
    use_all_tls_variables();
    
    /* Print results to prevent optimization */
    printf("TLS weak address: %p\n", (void*)tls_addresses[0]);
    printf("TLS hidden address: %p\n", (void*)tls_addresses[1]);
    printf("TLS checksum: %d\n", tls_values[0]);
    printf("TLS in section value: %d\n", tls_values[1]);
    
    /* Check if we're using emulated TLS */
    #ifndef __HAVE_TLS
    printf("Using emulated TLS (no native TLS support)\n");
    #endif
    
    /* Additional check: modify and read back TLS values */
    tls_weak = 1000;
    tls_hidden = 2000;
    printf("Modified TLS values: %d, %d\n", tls_weak, tls_hidden);
    
    return 0;
}
