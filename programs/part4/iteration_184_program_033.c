/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Force emulated TLS handling */
#ifdef __GNUC__
#define TLS_ATTRS __attribute__((used))
#else
#define TLS_ATTRS
#endif

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

/* 6. TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible_default = 6;

/* 7. Uninitialized TLS (should be common) */
__thread int tls_uninit;

/* 8. DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use dllimport-like attribute if supported */
#if __has_attribute(dllimport)
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Fallback: just a regular TLS */
__thread int tls_dllimport = 8;
#endif
#endif

/* 9. Combination: weak + hidden */
__attribute__((weak, visibility("hidden"))) __thread int tls_weak_hidden;

/* 10. Extern definition (matches declaration above) */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    if (tls_weak) {
        tls_weak = 100;
    }
    
    tls_hidden = tls_visible_default + tls_static;
    
    /* Ensure tls_uninit is not optimized away */
    tls_uninit = tls_default;
    
    /* Use dllimport TLS if available */
    tls_dllimport = 99;
    
    tls_weak_hidden = 77;
}

/* Another helper that takes addresses */
void use_tls_pointers(void) {
    /* Take addresses to inhibit optimizations */
    int *p1 = &tls_default;
    int *p2 = &tls_static;
    int *p3 = &tls_extern;
    int *p4 = &tls_weak;
    int *p5 = &tls_hidden;
    int *p6 = &tls_visible_default;
    int *p7 = &tls_uninit;
    int *p8 = &tls_dllimport;
    int *p9 = &tls_weak_hidden;
    
    /* Use pointers to create side effects */
    if (p1 && p2 && p3) {
        *p1 += 1;
        *p2 += 1;
        *p3 += 1;
    }
    
    /* Prevent unused variable warnings */
    (void)p4; (void)p5; (void)p6; (void)p7; (void)p8; (void)p9;
}

int main(void) {
    int sum = 0;
    
    /* Initial use of TLS variables */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper */
    modify_tls();
    
    /* Use pointers to TLS */
    use_tls_pointers();
    
    /* Compute final result using all TLS vars */
    int result = tls_default + tls_static + tls_extern + 
                 tls_weak + tls_hidden + tls_visible_default +
                 tls_uninit + tls_dllimport + tls_weak_hidden;
    
    printf("Final result: %d\n", result);
    
    /* Additional complex expression to ensure all TLS is used */
    if (tls_default > 0) {
        tls_static = tls_extern * tls_hidden;
    }
    
    /* Access weak TLS through pointer indirection */
    int *weak_ptr = &tls_weak;
    *weak_ptr += 1000;
    
    printf("Weak TLS value: %d\n", tls_weak);
    
    return 0;
}

/* Force generation of TLS initialization */
void __attribute__((constructor)) init_tls(void) {
    /* Access all TLS variables in constructor */
    tls_default = 1;
    tls_static = 2;
    tls_extern = 3;
    tls_weak = 4;
    tls_hidden = 5;
    tls_visible_default = 6;
    tls_uninit = 7;
    tls_dllimport = 8;
    tls_weak_hidden = 9;
}
