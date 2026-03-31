/* tls_emutest.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS by targeting older architecture */
#ifdef __x86_64__
#undef __x86_64__
#endif

/* Disable native TLS if possible */
#ifdef __HAVE_TLS
#undef __HAVE_TLS
#endif

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of TLS accesses */
volatile void *volatile tls_ptrs[10];
volatile int volatile tls_values[10];
int global_counter = 0;

/* Helper to prevent dead code elimination */
__attribute__((noinline, used))
static void use_tls_variable(void *ptr, int value) {
    tls_ptrs[global_counter] = ptr;
    tls_values[global_counter] = value;
    global_counter++;
}

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* 4. Initialized TLS variable - ensures it's not optimized away */
__thread int tls_initialized = 42;

/* 5. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 6. TLS with used attribute - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. Static TLS inside function - different DECL_CONTEXT */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 100;
    use_tls_variable(&tls_function_static, tls_function_static);
}

/* 8. TLS with section attribute - additional complexity */
__thread int tls_in_section __attribute__((section(".tls_data"))) = 200;

/* 9. TLS with aligned attribute */
__thread int tls_aligned __attribute__((aligned(64))) = 300;

/* For Windows/MinGW DLL import simulation */
#ifdef _WIN32
/* 10. DLL Import TLS - triggers DECL_DLLIMPORT_P */
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, use another visibility */
__thread int tls_imported __attribute__((visibility("protected")));
#endif

/* ===== TLS USAGE FUNCTIONS ===== */

__attribute__((noinline, used))
static void process_all_tls(void) {
    int checksum = 0;
    
    /* Use weak TLS */
    tls_weak = 1;
    checksum += tls_weak;
    use_tls_variable(&tls_weak, tls_weak);
    
    /* Use hidden TLS */
    tls_hidden = 2;
    checksum += tls_hidden;
    use_tls_variable(&tls_hidden, tls_hidden);
    
    /* Use common TLS */
    tls_common = 3;
    checksum += tls_common;
    use_tls_variable(&tls_common, tls_common);
    
    /* Use initialized TLS */
    checksum += tls_initialized;
    use_tls_variable(&tls_initialized, tls_initialized);
    
    /* Use preserved TLS */
    tls_preserved = 5;
    checksum += tls_preserved;
    use_tls_variable(&tls_preserved, tls_preserved);
    
    /* Use section TLS */
    checksum += tls_in_section;
    use_tls_variable(&tls_in_section, tls_in_section);
    
    /* Use aligned TLS */
    checksum += tls_aligned;
    use_tls_variable(&tls_aligned, tls_aligned);
    
    /* Use imported TLS */
    tls_imported = 8;
    checksum += tls_imported;
    use_tls_variable(&tls_imported, tls_imported);
    
    /* Force external TLS reference */
    if (&tls_external != NULL) {
        checksum += 9;
    }
    
    /* Store checksum in TLS to ensure all are used */
    tls_common = checksum;
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    printf("Testing emulated TLS with various attributes...\n");
    
    /* Process all TLS variables */
    process_all_tls();
    
    /* Call function with static TLS */
    function_with_static_tls();
    
    /* Force different TLS addresses to be computed */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Create pointer comparisons to force TLS references */
    volatile void *ptr1 = &tls_weak;
    volatile void *ptr2 = &tls_hidden;
    volatile void *ptr3 = &tls_common;
    volatile void *ptr4 = &tls_initialized;
    
    (void)ptr1; (void)ptr2; (void)ptr3; (void)ptr4;
    
    /* Compute and print a result based on TLS values */
    int result = tls_weak + tls_hidden + tls_common + 
                 tls_initialized + tls_preserved +
                 tls_in_section + tls_aligned + tls_imported;
    
    printf("TLS result: %d\n", result);
    printf("Processed %d TLS variables\n", global_counter);
    
    return result == 0 ? 1 : 0;
}

/* ===== EXTERNAL TLS DEFINITION (for multi-TU simulation) ===== */
/* This would normally be in a separate file, but we include it here
   with a guard to simulate the multi-TU scenario */
#ifdef COMPILE_EXTERNAL_DEF
__thread int tls_external = 12345;
#endif
