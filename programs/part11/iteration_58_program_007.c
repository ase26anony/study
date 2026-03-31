/* tls_emutls_test.c
 * Test program to cover GCC's emulated TLS initialization lines 295-304
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread tls_emutls_test.c -o tls_test
 * For 32-bit: gcc -O2 -m32 -ftls-model=emulated -fno-builtin tls_emutls_test.c -o tls_test32
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent dead code elimination */
volatile void *volatile tls_addresses[10];
volatile int volatile tls_values[10];
int global_checksum = 0;

/* ===== TLS VARIABLES WITH VARIOUS ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. TLS variable with used attribute - may influence DECL_PRESERVE_P */
__thread int tls_used __attribute__((used));

/* 5. Public TLS variable with initializer - affects TREE_PUBLIC */
__thread int tls_public = 42;

/* 6. External TLS declaration - triggers DECL_EXTERNAL (will be defined in main) */
extern __thread int tls_external;

/* 7. Define the external TLS variable */
__thread int tls_external = 100;

/* 8. TLS variable in function scope - affects DECL_CONTEXT */
static void function_with_tls(void) {
    static __thread int tls_in_function = 777;
    tls_addresses[7] = (void*)&tls_in_function;
    tls_values[7] = tls_in_function;
}

/* 9. TLS variable with noinline function usage - affects DECL_PRESERVE_P */
__thread int tls_preserved;

/* 10. For Windows DLL import simulation (guarded) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar concept on non-Windows */
__thread int tls_imported __attribute__((weak));
#endif

/* ===== HELPER FUNCTIONS ===== */

/* Force noinline to ensure TLS variables are preserved */
__attribute__((noinline, used)) 
static void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_common;
    tls_addresses[3] = (void*)&tls_used;
    tls_addresses[4] = (void*)&tls_public;
    tls_addresses[5] = (void*)&tls_external;
    tls_addresses[6] = (void*)&tls_preserved;
    tls_addresses[8] = (void*)&tls_imported;
    
    /* Initialize and use TLS variables */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_used = 4;
    tls_preserved = 8;
    
    /* Modify public TLS */
    tls_public += 1;
    
    /* Use imported TLS */
    tls_imported = 9;
    
    /* Store values for checksum */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = tls_used;
    tls_values[4] = tls_public;
    tls_values[5] = tls_external;
    tls_values[6] = tls_preserved;
    tls_values[8] = tls_imported;
}

/* Another function to ensure TLS context is used */
__attribute__((noinline))
static void compute_tls_checksum(void) {
    int sum = 0;
    
    /* Force compiler to actually access TLS variables */
    for (int i = 0; i < 9; i++) {
        if (i != 7) { /* Skip function-scoped TLS index */
            sum += tls_values[i];
        }
    }
    
    /* Use function-scoped TLS */
    function_with_tls();
    sum += tls_values[7];
    
    global_checksum = sum;
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    /* Runtime check for emulated TLS */
    printf("Testing emulated TLS coverage...\n");
    
    /* Use TLS variables through helper functions */
    use_tls_variables();
    compute_tls_checksum();
    
    /* Force referencing of variables in main */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print checksum to prevent optimization */
    printf("TLS checksum: %d\n", global_checksum);
    
    /* Print individual TLS values */
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_common: %d\n", tls_common);
    printf("tls_used: %d\n", tls_used);
    printf("tls_public: %d\n", tls_public);
    printf("tls_external: %d\n", tls_external);
    printf("tls_preserved: %d\n", tls_preserved);
    printf("tls_imported: %d\n", tls_imported);
    
    return global_checksum > 0 ? 0 : 1;
}
