/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS emulation attribute copying logic */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

#include <stdio.h>

/* Force emulated TLS by declaring with __thread */
/* Various TLS declarations with different attributes */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static (internal) linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* External definition */
__thread int tls_extern = 3;

/* Weak symbol */
__attribute__((weak)) __thread int tls_weak;

/* Hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* Default visibility explicitly */
__attribute__((visibility("default"))) __thread int tls_default_vis = 5;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 6;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but declare it anyway */
__thread int tls_dllimport;
#endif

/* Common linkage simulation */
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Initialize weak TLS if not already defined */
    if (tls_weak == 0) {
        tls_weak = 100;
    }
    
    tls_hidden = tls_default + tls_static;
    tls_used++;
}

/* Dummy function to consume address of TLS variable */
void use_tls_address(int *addr) {
    /* Prevent optimization */
    static volatile int sink;
    sink = *addr;
}

int main(void) {
    int result = 0;
    
    /* Initialize some TLS variables */
    tls_common = 42;
    tls_dllimport = 7;
    
    /* Use TLS variables in main */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Compute new result */
    result = tls_default + tls_static + tls_extern + tls_weak + 
             tls_hidden + tls_used + tls_common;
    
    printf("Modified sum: %d\n", result);
    
    /* Take address of TLS variables to ensure they're fully processed */
    int *addr1 = &tls_default;
    int *addr2 = &tls_static;
    int *addr3 = &tls_hidden;
    
    use_tls_address(addr1);
    use_tls_address(addr2);
    use_tls_address(addr3);
    
    /* Conditional use to prevent dead code elimination */
    if (tls_default > 0) {
        printf("tls_default is positive: %d\n", tls_default);
    }
    
    /* Return computed result to prevent optimization */
    return result > 100 ? 0 : 1;
}
