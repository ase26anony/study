/* tls_main.c - Main test program for emulated TLS attribute copying */

/* Force emulated TLS mode */
#ifndef __HAVE_TLS
#define __HAVE_TLS 0
#endif

#if __HAVE_TLS
#error "This test requires emulated TLS, not native TLS"
#endif

#include <stdio.h>
#include <stdint.h>

/* Prevent dead code elimination */
volatile void *tls_addresses[10];
volatile int tls_values[10];
int checksum = 0;

/* Helper function to use TLS variables - marked noinline to prevent optimization */
__attribute__((noinline, used, retain))
void use_tls_variables(void) {
    /* 1. Weak TLS variable */
    extern __thread int tls_weak __attribute__((weak));
    tls_addresses[0] = (void*)&tls_weak;
    if (&tls_weak != NULL) {
        tls_weak = 100;
        tls_values[0] = tls_weak;
    }
    
    /* 2. TLS with explicit visibility (hidden) */
    __thread int tls_hidden __attribute__((visibility("hidden")));
    tls_addresses[1] = (void*)&tls_hidden;
    tls_hidden = 200;
    tls_values[1] = tls_hidden;
    
    /* 3. Common TLS variable (tentative definition) */
    __thread int tls_common;  /* No initializer = common linkage */
    tls_addresses[2] = (void*)&tls_common;
    tls_common = 300;
    tls_values[2] = tls_common;
    
    /* 4. External TLS declaration */
    extern __thread int tls_external;
    tls_addresses[3] = (void*)&tls_external;
    tls_values[3] = tls_external;
    
    /* 5. Public TLS with used attribute (affects DECL_PRESERVE_P) */
    __thread int tls_public __attribute__((used));
    tls_addresses[4] = (void*)&tls_public;
    tls_public = 500;
    tls_values[4] = tls_public;
    
    /* 6. TLS with protected visibility */
    __thread int tls_protected __attribute__((visibility("protected")));
    tls_addresses[5] = (void*)&tls_protected;
    tls_protected = 600;
    tls_values[5] = tls_protected;
    
    /* 7. Static TLS (non-public, tests DECL_CONTEXT) */
    static __thread int tls_static;
    tls_addresses[6] = (void*)&tls_static;
    tls_static = 700;
    tls_values[6] = tls_static;
    
    /* 8. TLS with both weak and visibility attributes */
    extern __thread int tls_weak_hidden __attribute__((weak, visibility("hidden")));
    tls_addresses[7] = (void*)&tls_weak_hidden;
    if (&tls_weak_hidden != NULL) {
        tls_weak_hidden = 800;
        tls_values[7] = tls_weak_hidden;
    }
    
    /* 9. Initialized TLS (non-common) */
    __thread int tls_init = 900;
    tls_addresses[8] = (void*)&tls_init;
    tls_values[8] = tls_init;
    
    /* 10. TLS in different namespace context (C++ style in C) */
    struct context {
        __thread int member_tls;
    };
    static struct context ctx;
    tls_addresses[9] = (void*)&ctx.member_tls;
    ctx.member_tls = 1000;
    tls_values[9] = ctx.member_tls;
}

/* Another function to ensure TLS variables are used in multiple contexts */
__attribute__((noinline))
int compute_tls_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        if (tls_addresses[i] != NULL) {
            sum += tls_values[i];
        }
    }
    return sum;
}

int main(void) {
    /* Use TLS variables */
    use_tls_variables();
    
    /* Compute checksum to prevent optimization */
    checksum = compute_tls_checksum();
    
    /* Force addresses to be taken and compared */
    __thread int tls_hidden __attribute__((visibility("hidden")));
    __thread int tls_common;
    
    if (&tls_hidden != &tls_common) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print something to ensure execution */
    printf("TLS test completed. Checksum: %d\n", checksum);
    
    return 0;
}
