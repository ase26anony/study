/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage, initialized */
static __thread int tls_static = 2;

/* 3. External TLS declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol, uninitialized */
__attribute__((weak)) __thread int tls_weak;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* 6. TLS with default visibility and used attribute */
__attribute__((used)) __thread int tls_used = 6;

/* 7. TLS with DLL import attribute (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, simulate with declspec for completeness */
__attribute__((visibility("default"))) __thread int tls_dllimport = 7;
#endif

/* 8. Common TLS (uninitialized, external linkage) */
__thread int tls_common;

/* Definition of the extern TLS variable */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Initialize uninitialized TLS if needed */
    if (tls_weak == 0) {
        tls_weak = 42;
    }
    
    tls_hidden = 100;
    tls_used++;
    tls_dllimport = 77;
    tls_common = 999;
}

/* Another helper that takes addresses of TLS variables */
void take_tls_addresses(void) {
    /* Taking addresses forces symbol references */
    int *p1 = &tls_default;
    int *p2 = &tls_static;
    int *p3 = &tls_extern;
    int *p4 = &tls_weak;
    int *p5 = &tls_hidden;
    
    /* Use pointers to create side effects */
    if (p1 && p2 && p3 && p4 && p5) {
        /* Prevent optimization */
        volatile int dummy = *p1 + *p2;
        (void)dummy;
    }
}

int main(void) {
    int sum = 0;
    
    /* Initialize uninitialized TLS variables */
    tls_hidden = 5;
    tls_common = 8;
    
    /* Use all TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    
    /* Call helper functions */
    modify_tls();
    take_tls_addresses();
    
    /* More usage after modification */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_used;
    sum += tls_dllimport;
    sum += tls_common;
    
    /* Print result to prevent dead code elimination */
    printf("TLS sum: %d\n", sum);
    printf("tls_default=%d, tls_static=%d, tls_extern=%d\n", 
           tls_default, tls_static, tls_extern);
    printf("tls_weak=%d, tls_hidden=%d, tls_used=%d\n",
           tls_weak, tls_hidden, tls_used);
    printf("tls_dllimport=%d, tls_common=%d\n",
           tls_dllimport, tls_common);
    
    return 0;
}
