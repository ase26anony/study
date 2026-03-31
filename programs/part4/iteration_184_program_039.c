/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* 1. Basic TLS with default visibility and used attribute */
__attribute__((used)) __thread int tls_default = 1;

/* 2. Static TLS with internal linkage */
static __thread int tls_static = 2;

/* 3. External declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* 6. TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_default_vis;

/* 7. DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use a different attribute to ensure DECL_DLLIMPORT_P is considered */
__thread int tls_dllimport __attribute__((weak));
#endif

/* 8. Common TLS (uninitialized, external linkage) */
__thread int tls_common;

/* 9. External definition (matches declaration above) */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS if available */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Hidden visibility TLS */
    tls_hidden = 42;
    
    /* Default visibility TLS */
    tls_default_vis = 99;
    
    /* Common TLS */
    tls_common = 77;
    
    /* DLL import simulation */
    tls_dllimport = 55;
}

/* Another helper to take addresses */
void* get_tls_addresses(void) {
    static void* addrs[8];
    
    /* Take addresses to prevent optimizations */
    addrs[0] = &tls_default;
    addrs[1] = &tls_static;
    addrs[2] = &tls_extern;
    addrs[3] = &tls_weak;
    addrs[4] = &tls_hidden;
    addrs[5] = &tls_default_vis;
    addrs[6] = &tls_common;
    addrs[7] = &tls_dllimport;
    
    return addrs;
}

int main(void) {
    int sum = 0;
    
    /* Initial values */
    printf("Initial TLS values:\n");
    printf("tls_default = %d\n", tls_default);
    printf("tls_static = %d\n", tls_static);
    printf("tls_extern = %d\n", tls_extern);
    
    /* Modify TLS in helper */
    modify_tls();
    
    /* Read modified values */
    sum = tls_default + tls_static + tls_extern;
    printf("After modification:\n");
    printf("tls_default = %d\n", tls_default);
    printf("tls_static = %d\n", tls_static);
    printf("tls_extern = %d\n", tls_extern);
    printf("Sum = %d\n", sum);
    
    /* Force address-taking to ensure symbols are needed */
    void* addrs = get_tls_addresses();
    (void)addrs; /* Prevent unused variable warning */
    
    /* Use hidden visibility TLS */
    tls_hidden++;
    printf("tls_hidden = %d\n", tls_hidden);
    
    /* Use default visibility TLS */
    tls_default_vis--;
    printf("tls_default_vis = %d\n", tls_default_vis);
    
    /* Use common TLS */
    tls_common += 10;
    printf("tls_common = %d\n", tls_common);
    
    /* Conditional use of weak TLS */
    if (&tls_weak) {
        printf("tls_weak = %d\n", tls_weak);
    }
    
    return sum > 0 ? 0 : 1;
}

/* Additional TLS in different linkage context */
__thread int tls_another = 123;
static __thread int tls_another_static = 456;
