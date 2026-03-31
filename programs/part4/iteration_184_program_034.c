/* This test case is designed to trigger the TLS emulation attribute copying
   logic in tree-emutls.cc, specifically lines 295-304. It uses various
   thread-local variables with different attributes to ensure all relevant
   declaration flags are propagated during emulated TLS setup. */

/* Force emulated TLS handling */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Helper function to modify TLS variables */
void modify_tls(void);

/* Plain external linkage TLS with initialization */
__thread int tls_default = 1;

/* Static TLS with initialization - internal linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS symbol - may be overridden */
__attribute__((weak)) __thread int tls_weak;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible_default;

/* TLS marked as used to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used_attr;

/* DLL import simulation (for DECL_DLLIMPORT_P testing) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but we'll keep a declaration */
extern __thread int tls_dllimport;
#endif

/* Definition of previously declared extern TLS */
__thread int tls_extern = 3;

/* Uninitialized TLS with external linkage */
__thread int tls_uninitialized;

/* Common TLS symbol (simulated via tentative definition) */
__thread int tls_common;

/* Function that operates on TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 3;
    tls_extern -= 1;
    
    /* Initialize uninitialized TLS */
    tls_uninitialized = 100;
    tls_common = 200;
    
    /* Use weak TLS if available */
    if (&tls_weak) {
        tls_weak = 50;
    }
    
    /* Use hidden visibility TLS */
    tls_hidden = tls_default + tls_static;
    
    /* Use the 'used' attributed TLS */
    tls_used_attr = tls_hidden * 2;
}

/* Dummy function that takes address of TLS variable */
void use_tls_address(int *ptr) {
    /* Prevent optimization */
    static volatile int sink;
    sink = *ptr;
}

int main(void) {
    int result = 0;
    
    /* Initial use of TLS variables */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    
    /* Call function that modifies TLS */
    modify_tls();
    
    /* Use TLS variables after modification */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    result += tls_uninitialized;
    result += tls_common;
    
    /* Take address of TLS variables to ensure they're fully processed */
    int *addr1 = &tls_default;
    int *addr2 = &tls_static;
    int *addr3 = &tls_hidden;
    
    /* Use addresses to prevent optimization */
    use_tls_address(addr1);
    use_tls_address(addr2);
    use_tls_address(addr3);
    
    /* Try to use weak TLS */
    if (&tls_weak) {
        result += tls_weak;
        use_tls_address(&tls_weak);
    }
    
    /* Use the 'used' attributed TLS */
    result += tls_used_attr;
    use_tls_address(&tls_used_attr);
    
    /* Print result to prevent dead code elimination */
    printf("TLS test result: %d\n", result);
    
    /* Additional printf to use all TLS variables */
    printf("TLS values: default=%d, static=%d, extern=%d, uninit=%d, common=%d\n",
           tls_default, tls_static, tls_extern, 
           tls_uninitialized, tls_common);
    
    return 0;
}

/* Simulate DLL import definition (would normally be in another file) */
#ifndef _WIN32
/* On non-Windows, provide a definition for the dllimport-simulated TLS */
__thread int tls_dllimport = 999;
#endif
