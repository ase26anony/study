/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS by using appropriate compiler flags:
   -femulated-tls or -ftls-model=emulated
   Target architectures without native TLS support (e.g., -march=armv7-a)
*/

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage */
static __thread int tls_static = 2;

/* 3. External TLS declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* 6. TLS with default visibility and used attribute */
__attribute__((visibility("default"))) __attribute__((used)) 
__thread int tls_visible_used = 6;

/* 7. Uninitialized TLS */
__thread int tls_uninit;

/* 8. DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can't truly test dllimport, but keep a placeholder */
__thread int tls_dllimport;
#endif

/* 9. Common TLS (uninitialized external) - DECL_COMMON should be set */
extern __thread int tls_common;

/* Definition of external TLS variable */
__thread int tls_extern = 3;

/* Definition of common TLS variable */
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    
    if (tls_weak == 0) {
        tls_weak = 42;  /* Initialize weak TLS if not already */
    }
    
    tls_hidden = tls_default + tls_static;
    tls_visible_used++;
    
    /* Take address to inhibit optimizations */
    int *addr1 = &tls_default;
    int *addr2 = &tls_hidden;
    
    /* Use addresses to create side effects */
    if (addr1 != addr2) {
        tls_uninit = (int)(addr1 < addr2);
    }
}

/* Another helper to take addresses of TLS variables */
void use_tls_addresses(void) {
    int *ptrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_visible_used,
        &tls_uninit,
        &tls_dllimport,
        &tls_common
    };
    
    /* Use pointers to prevent optimization */
    int sum = 0;
    for (int i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
        if (ptrs[i]) {
            sum += *ptrs[i];
        }
    }
    
    /* Store result in a TLS variable */
    tls_uninit = sum % 100;
}

int main(void) {
    /* Initialize some TLS variables */
    tls_weak = 5;
    tls_hidden = 7;
    tls_dllimport = 8;
    tls_common = 9;
    
    /* Use TLS variables in main */
    int result = tls_default + tls_static + tls_extern;
    printf("Initial sum: %d\n", result);
    
    /* Call helper functions */
    modify_tls();
    use_tls_addresses();
    
    /* Compute final result using all TLS variables */
    result = tls_default + tls_static + tls_extern + tls_weak + 
             tls_hidden + tls_visible_used + tls_uninit + tls_dllimport + 
             tls_common;
    
    printf("Final TLS sum: %d\n", result);
    
    /* Additional use to ensure all variables are referenced */
    printf("TLS addresses: %p %p %p\n", 
           (void*)&tls_default, 
           (void*)&tls_hidden, 
           (void*)&tls_visible_used);
    
    return result > 0 ? 0 : 1;
}
