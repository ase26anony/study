/* test-emutls-attributes.c
 * 
 * This program tests GCC's emulated TLS initialization by creating
 * thread-local variables with diverse attributes that should trigger
 * the property copying in emutls_decl() function (lines 295-304).
 */

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

/* Prevent optimization of TLS variables */
#define USED_VAR __attribute__((used))
#define NOINLINE __attribute__((noinline))

/* Global volatile array to store TLS addresses and prevent optimization */
volatile void *tls_addresses[10];
volatile int tls_values[10];

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;

/* 3. Common TLS variable - tentative definition, may trigger DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - will be defined later, triggers TREE_PUBLIC and DECL_EXTERNAL */
extern __thread int tls_external;

/* 5. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used)) = 4;

/* 6. Public TLS variable with used attribute */
__thread int tls_public USED_VAR = 5;

/* 7. Static TLS variable (non-public context) - affects DECL_CONTEXT */
static __thread int tls_static = 6;

/* Windows-specific DLL import attribute */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__attribute__((dllimport)) __thread int tls_imported;
#else
/* For non-Windows, create a dummy declaration */
extern __thread int tls_imported;
#endif

/* Function to ensure TLS variables are used in non-optimizable ways */
NOINLINE static void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = &tls_weak;
    tls_addresses[1] = &tls_hidden;
    tls_addresses[2] = &tls_common;
    tls_addresses[3] = &tls_external;
    tls_addresses[4] = &tls_preserved;
    tls_addresses[5] = &tls_public;
    tls_addresses[6] = &tls_static;
    
    /* Use TLS variables in computations */
    tls_common = tls_weak + tls_hidden;
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_common;
    tls_values[3] = tls_preserved;
    tls_values[4] = tls_public;
    tls_values[5] = tls_static;
    
    /* Force compiler to keep all variables */
    asm volatile("" : : "r"(&tls_weak), "r"(&tls_hidden), "r"(&tls_common));
}

/* Another function with different context */
NOINLINE static int compute_tls_checksum(void) {
    int sum = 0;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_preserved;
    sum += tls_public;
    sum += tls_static;
    
    /* Create a pointer chain to prevent optimization */
    volatile int * volatile ptr = &tls_weak;
    tls_addresses[7] = ptr;
    
    return sum;
}

/* Define the external TLS variable (simulating multi-TU scenario) */
__thread int tls_external = 7;

/* For Windows DLL import simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int tls_imported = 8;
#elif defined(__MINGW32__)
__attribute__((dllexport)) __thread int tls_imported = 8;
#else
__thread int tls_imported = 8;
#endif

int main(void) {
    int i, checksum;
    
    /* Initialize common TLS variable */
    tls_common = 3;
    
    /* Use TLS variables in helper function */
    use_tls_variables();
    
    /* Compute checksum using TLS variables */
    checksum = compute_tls_checksum();
    
    /* Force use of all TLS variables in main */
    tls_addresses[8] = &tls_imported;
    tls_values[6] = tls_imported;
    
    /* Ensure addresses differ (prevents optimization) */
    if (&tls_weak != &tls_hidden) {
        checksum += 1;
    }
    
    /* Use checksum in a way compiler can't eliminate */
    volatile int result = checksum;
    
    /* Print something to ensure execution */
    printf("TLS test completed. Checksum: %d\n", checksum);
    printf("TLS addresses:\n");
    for (i = 0; i < 9; i++) {
        if (tls_addresses[i]) {
            printf("  %p\n", (void*)tls_addresses[i]);
        }
    }
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}

/* Additional file for multi-TU testing (compile separately):
 * 
 * File: tls-external-def.c
 * 
 * __thread int tls_external = 7;
 * 
 * Compile with: gcc -c -O2 -ftls-model=emulated tls-external-def.c
 * Link with: gcc -O2 -ftls-model=emulated test-emutls-attributes.c tls-external-def.o
 */
