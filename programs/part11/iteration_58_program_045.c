/* test-emutls-coverage.c
 * Comprehensive test to cover emulated TLS attribute copying in GCC's tree-emutls.cc
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread test-emutls-coverage.c -o test-emutls
 * For 32-bit: gcc -O2 -m32 -ftls-model=emulated -fno-builtin test-emutls-coverage.c -o test-emutls-32
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent dead code elimination */
volatile void* volatile tls_addresses[10];
volatile int volatile tls_values[10];
int global_counter = 0;

/* Helper function marked noinline to prevent optimization */
__attribute__((noinline)) void use_tls_variables(void) {
    /* 1. Weak TLS variable - triggers DECL_WEAK copying */
    __thread int tls_weak __attribute__((weak));
    tls_weak = 100 + global_counter++;
    tls_addresses[0] = (void*)&tls_weak;
    tls_values[0] = tls_weak;
    
    /* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
    __thread int tls_hidden __attribute__((visibility("hidden")));
    tls_hidden = 200 + global_counter++;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_values[1] = tls_hidden;
    
    /* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
    __thread int tls_common;
    tls_common = 300 + global_counter++;
    tls_addresses[2] = (void*)&tls_common;
    tls_values[2] = tls_common;
    
    /* 4. Public TLS variable with used attribute - triggers TREE_PUBLIC and potentially DECL_PRESERVE_P */
    __thread int tls_public __attribute__((used));
    tls_public = 400 + global_counter++;
    tls_addresses[3] = (void*)&tls_public;
    tls_values[3] = tls_public;
    
    /* 5. External TLS declaration (simulated with extern) - triggers DECL_EXTERNAL */
    extern __thread int tls_external;
    tls_addresses[4] = (void*)&tls_external;
    tls_values[4] = tls_external;
    
    /* 6. TLS variable with preserve attribute - triggers DECL_PRESERVE_P */
    __thread int tls_preserve __attribute__((used));
    tls_preserve = 600 + global_counter++;
    tls_addresses[5] = (void*)&tls_preserve;
    tls_values[5] = tls_preserve;
    
    /* Force computation using all TLS variables to prevent optimization */
    int checksum = tls_weak + tls_hidden + tls_common + tls_public + tls_external + tls_preserve;
    tls_values[6] = checksum;
    
    /* Force context by taking addresses */
    if (&tls_weak != &tls_hidden) {
        tls_values[7] = 1;
    }
}

/* Another function to create different DECL_CONTEXT */
static void nested_function(void) {
    /* TLS in function scope - different DECL_CONTEXT */
    static __thread int tls_in_function;
    tls_in_function = 700 + global_counter++;
    tls_addresses[8] = (void*)&tls_in_function;
    tls_values[8] = tls_in_function;
}

/* Windows-specific DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar concept with weak external */
extern __thread int tls_imported __attribute__((weak));
#endif

/* External TLS definition (for the extern declaration) */
__thread int tls_external = 42;

/* Weak imported TLS definition */
__thread int tls_imported = 99;

int main(void) {
    printf("Testing emulated TLS attribute coverage...\n");
    
    /* Use the TLS variables in main to ensure they're processed */
    use_tls_variables();
    nested_function();
    
    /* Use Windows-style import if compiled for Windows */
#ifdef _WIN32
    tls_addresses[9] = (void*)&tls_imported;
    tls_values[9] = tls_imported;
#else
    tls_addresses[9] = (void*)&tls_imported;
    tls_values[9] = tls_imported;
#endif
    
    /* Compute and print a checksum from all TLS values */
    int final_checksum = 0;
    for (int i = 0; i < 10; i++) {
        final_checksum += (int)(uintptr_t)tls_addresses[i] + tls_values[i];
    }
    
    printf("TLS checksum: %d\n", final_checksum);
    printf("All TLS addresses captured, forcing emutls_decl processing.\n");
    
    /* Check if we're using emulated TLS */
#ifdef __HAVE_TLS
    printf("Native TLS support detected\n");
#else
    printf("Emulated TLS mode expected\n");
#endif
    
    return 0;
}
