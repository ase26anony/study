/* tls_emutest.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode */
#ifndef __HAVE_TLS
#pragma message "Using emulated TLS"
#endif

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* Prevent optimization */
#define USED __attribute__((used))
#define NOINLINE __attribute__((noinline))
#define PRESERVE __attribute__((used, noinline))

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;

/* 3. Common TLS variable - tentative definition, may trigger DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers TREE_PUBLIC and DECL_EXTERNAL */
extern __thread int tls_external;

/* 5. DLL Import TLS (Windows-specific) */
#ifdef _WIN32
DLL_IMPORT __thread int tls_imported;
#else
/* On non-Windows, simulate with visibility */
__thread int tls_imported __attribute__((visibility("default")));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved PRESERVE = 5;

/* 7. Public TLS with initializer */
__thread int tls_public = 6;

/* 8. Static TLS inside function - different DECL_CONTEXT */
static void func_with_tls(void) {
    static __thread int tls_in_func = 7;
    volatile int *p = &tls_in_func;
    global_checksum += *p;
}

/* Helper function that uses all TLS variables */
NOINLINE static void use_all_tls(void) {
    /* Take addresses to force TLS instantiation */
    volatile int *ptrs[] = {
        &tls_weak,
        &tls_hidden,
        &tls_common,
        &tls_preserved,
        &tls_public,
        NULL
    };
    
    /* Use values to prevent optimization */
    tls_common = 3;  /* Initialize common TLS */
    
    int sum = 0;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_preserved;
    sum += tls_public;
    
    /* Force external TLS reference */
    extern void use_external_tls(void);
    use_external_tls();
    
    /* Store result to prevent optimization */
    global_checksum = sum;
    
    /* Force different addresses check */
    if (&tls_weak != &tls_hidden) {
        global_checksum += 1000;
    }
    
    /* Call function with internal TLS */
    func_with_tls();
}

/* External TLS definition (simulate multi-TU) */
__thread int tls_external = 42;

/* Function using external TLS */
void use_external_tls(void) {
    volatile int *p = &tls_external;
    global_checksum += *p;
}

/* Main function */
int main(void) {
    /* Initialize before use */
    tls_common = 3;
    
    /* Use all TLS variables */
    use_all_tls();
    
    /* Additional direct usage */
    tls_weak++;
    tls_hidden *= 2;
    
    /* Create observable side effect */
    int result = tls_weak + tls_hidden + tls_common + tls_preserved + tls_public;
    
    /* Print to prevent optimization */
    volatile int output = result + global_checksum;
    
    /* Return checksum */
    return output & 0xFF;
}

/* Additional file for multi-TU simulation */
#ifdef COMPILE_SECOND_FILE
/* tls_emutest2.c - Second compilation unit */

/* Define the external TLS variable */
__thread int tls_external = 42;

/* Define Windows-exported TLS */
#ifdef _WIN32
DLL_EXPORT __thread int tls_imported = 99;
#endif
#endif
