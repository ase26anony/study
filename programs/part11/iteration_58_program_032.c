/* tls_emutest.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode */
#ifndef __HAVE_TLS
#define __HAVE_TLS 0
#endif

#if __HAVE_TLS
#warning "Native TLS support detected - may not trigger emulated TLS paths"
#endif

/* For Windows DLL import/export attributes */
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#define DLLIMPORT
#endif

/* Prevent optimization of TLS accesses */
volatile void* tls_addresses[10];
volatile int tls_values[10];
int checksum_result = 0;

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;
__thread int tls_default __attribute__((visibility("default"))) = 3;

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;  /* No initializer */

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) */
#ifdef _WIN32
DLLIMPORT __thread int tls_imported;
#else
/* On non-Windows, use weak external as alternative */
extern __thread int tls_imported __attribute__((weak));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used)) = 6;

/* 7. Public TLS with used attribute */
__thread int tls_public_used __attribute__((used)) = 7;

/* 8. Static TLS (non-public) to test different DECL_CONTEXT */
static __thread int tls_static = 8;

/* 9. TLS in different linkage categories */
__thread int tls_init_zero = 0;
__thread int tls_init_nonzero = 42;

/* ========== HELPER FUNCTIONS ========== */

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline, used)) 
static void process_tls_variables(void) {
    /* Take addresses of all TLS variables - prevents dead code elimination */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_default;
    tls_addresses[3] = (void*)&tls_common;
    tls_addresses[4] = (void*)&tls_external;
    tls_addresses[5] = (void*)&tls_imported;
    tls_addresses[6] = (void*)&tls_preserved;
    tls_addresses[7] = (void*)&tls_public_used;
    tls_addresses[8] = (void*)&tls_static;
    tls_addresses[9] = (void*)&tls_init_nonzero;
    
    /* Use values to prevent optimization */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_default;
    tls_values[3] = tls_common;
    tls_values[4] = tls_external;
    tls_values[5] = tls_imported;
    tls_values[6] = tls_preserved;
    tls_values[7] = tls_public_used;
    tls_values[8] = tls_static;
    tls_values[9] = tls_init_nonzero;
    
    /* Simple computation using TLS variables */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += tls_values[i];
    }
    
    /* Store result in global to prevent optimization */
    checksum_result = sum;
}

/* Another function to ensure TLS variables are referenced in multiple contexts */
__attribute__((noinline))
static void modify_tls_variables(void) {
    tls_weak += 1;
    tls_hidden *= 2;
    tls_default -= 1;
    tls_common = 100;
    tls_static = tls_static * 2 + 1;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 5;
    
    /* Process TLS variables */
    process_tls_variables();
    
    /* Modify them to ensure they're real variables */
    modify_tls_variables();
    
    /* Force references to all TLS variables */
    if (&tls_weak != &tls_hidden) {
        /* This condition is always true, but compiler doesn't know */
        checksum_result += (int)((long)&tls_weak % 256);
    }
    
    /* Use all TLS variables in observable ways */
    printf("TLS weak address: %p, value: %d\n", 
           (void*)&tls_weak, tls_weak);
    printf("TLS hidden address: %p, value: %d\n", 
           (void*)&tls_hidden, tls_hidden);
    printf("TLS common address: %p, value: %d\n", 
           (void*)&tls_common, tls_common);
    printf("TLS static address: %p, value: %d\n", 
           (void*)&tls_static, tls_static);
    
    /* Final checksum */
    process_tls_variables();
    printf("Checksum: %d\n", checksum_result);
    
    return checksum_result > 0 ? 0 : 1;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This would normally be in a separate file, but we include it here
   with a guard to simulate multi-file compilation */
#ifndef NO_EXTERNAL_DEF
__thread int tls_external = 99;

/* Weak definition for imported TLS */
#ifdef _WIN32
__thread int tls_imported = 77;
#else
__thread int tls_imported __attribute__((weak)) = 77;
#endif
#endif
