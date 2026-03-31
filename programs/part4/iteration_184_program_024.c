/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Force declaration attributes to be set on TLS variables */

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage */
static __thread int tls_static = 2;

/* 3. Extern declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 4;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* 6. TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_default_vis = 6;

/* 7. TLS marked as used to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* 8. Uninitialized TLS */
__thread int tls_uninit;

/* 9. Definition of extern TLS variable */
__thread int tls_extern = 3;

/* For Windows-like DLL import simulation */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Simulate dllimport-like attribute for testing */
__attribute__((weak)) __thread int tls_dllimport = 9;
#endif

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_static += 10;
    tls_hidden *= 2;
    tls_used = tls_default + tls_extern;
    
    /* Take address to inhibit optimizations */
    int *ptr = &tls_hidden;
    *ptr += 1;
    
    /* Use weak TLS */
    if (&tls_weak) {
        tls_weak = 100;
    }
}

/* Another function taking TLS address */
void use_tls_address(int **addr) {
    *addr = &tls_default_vis;
}

int main(void) {
    int result = 0;
    
    /* Initialize uninitialized TLS */
    tls_uninit = 8;
    
    /* Use all TLS variables in main */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden;
    result += tls_default_vis;
    result += tls_used;
    result += tls_uninit;
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate after modification */
    result = tls_default + tls_static + tls_extern + tls_weak + 
             tls_hidden + tls_default_vis + tls_used + tls_uninit;
    
    printf("Modified sum: %d\n", result);
    
    /* Take address of TLS variables */
    int *addr1 = &tls_default;
    int *addr2 = &tls_static;
    int *addr3 = &tls_extern;
    
    /* Use addresses to create side effects */
    *addr1 += 1;
    *addr2 += 2;
    *addr3 += 3;
    
    /* Get address via function */
    int *addr4 = NULL;
    use_tls_address(&addr4);
    if (addr4) {
        *addr4 = 99;
    }
    
    /* Final calculation */
    result = tls_default + tls_static + tls_extern + tls_weak + 
             tls_hidden + tls_default_vis + tls_used + tls_uninit;
    
    printf("Final sum: %d\n", result);
    
    /* Use dllimport-like TLS */
    tls_dllimport = result % 50;
    printf("DLL import sim value: %d\n", tls_dllimport);
    
    return 0;
}
