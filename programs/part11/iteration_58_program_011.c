/* emutls_test.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && __GNUC__ >= 4
#pragma GCC tls_model emulated
#endif

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of TLS accesses */
volatile void *volatile tls_addresses[10];
volatile int volatile tls_values[10];
int checksum = 0;

/* Helper function to use TLS variables - marked noinline to prevent optimization */
__attribute__((noinline, used))
static void use_tls_variables(void) {
    int idx = 0;
    
    /* 1. Weak TLS variable */
    __thread int tls_weak __attribute__((weak));
    tls_weak = 100;
    tls_addresses[idx] = (void*)&tls_weak;
    tls_values[idx] = tls_weak;
    idx++;
    
    /* 2. TLS with explicit visibility (hidden) */
    __thread int tls_hidden __attribute__((visibility("hidden")));
    tls_hidden = 200;
    tls_addresses[idx] = (void*)&tls_hidden;
    tls_values[idx] = tls_hidden;
    idx++;
    
    /* 3. Common TLS variable (tentative definition) */
    __thread int tls_common;  /* No initializer = tentative definition */
    tls_common = 300;
    tls_addresses[idx] = (void*)&tls_common;
    tls_values[idx] = tls_common;
    idx++;
    
    /* 4. External TLS declaration - will be defined in this file */
    extern __thread int tls_external;
    tls_addresses[idx] = (void*)&tls_external;
    tls_values[idx] = tls_external;
    idx++;
    
    /* 5. Public TLS variable with used attribute */
    __thread int tls_public __attribute__((used));
    tls_public = 500;
    tls_addresses[idx] = (void*)&tls_public;
    tls_values[idx] = tls_public;
    idx++;
    
    /* 6. TLS variable with preserve attribute (via retain in newer GCC) */
    #if __has_attribute(retain)
    __thread int tls_preserved __attribute__((retain));
    #else
    __thread int tls_preserved __attribute__((used));
    #endif
    tls_preserved = 600;
    tls_addresses[idx] = (void*)&tls_preserved;
    tls_values[idx] = tls_preserved;
    idx++;
    
    /* 7. Static TLS variable (non-public context) */
    static __thread int tls_static;
    tls_static = 700;
    tls_addresses[idx] = (void*)&tls_static;
    tls_values[idx] = tls_static;
    idx++;
    
    /* 8. Initialized TLS variable */
    __thread int tls_init = 800;
    tls_addresses[idx] = (void*)&tls_init;
    tls_values[idx] = tls_init;
    idx++;
    
    /* Calculate checksum to ensure all are used */
    for (int i = 0; i < idx; i++) {
        checksum += tls_values[i];
    }
}

/* Another function to ensure TLS variables are used in multiple contexts */
__attribute__((noinline))
static void modify_tls_variables(void) {
    /* Access the external TLS variable */
    extern __thread int tls_external;
    tls_external += 50;
    
    /* Force different addresses */
    __thread int tls_weak __attribute__((weak));
    __thread int tls_hidden __attribute__((visibility("hidden")));
    
    if (&tls_weak != &tls_hidden) {
        checksum += 1;  /* Ensure comparison isn't optimized away */
    }
}

/* Define the external TLS variable */
__thread int tls_external = 400;

/* For Windows DLL import simulation */
#ifdef _WIN32
/* Simulate DLL import/export scenario */
#ifdef BUILD_DLL
__declspec(dllexport) __thread int tls_exported = 900;
#else
__declspec(dllimport) __thread int tls_imported;
#endif
#endif

int main(void) {
    /* Use TLS variables */
    use_tls_variables();
    modify_tls_variables();
    
    /* Print results to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    /* Force all TLS variables to be referenced */
    __thread int tls_weak __attribute__((weak));
    __thread int tls_hidden __attribute__((visibility("hidden")));
    __thread int tls_common;
    extern __thread int tls_external;
    __thread int tls_public __attribute__((used));
    #if __has_attribute(retain)
    __thread int tls_preserved __attribute__((retain));
    #else
    __thread int tls_preserved __attribute__((used));
    #endif
    static __thread int tls_static;
    __thread int tls_init = 800;
    
    printf("TLS addresses:\n");
    printf("  weak: %p\n", (void*)&tls_weak);
    printf("  hidden: %p\n", (void*)&tls_hidden);
    printf("  common: %p\n", (void*)&tls_common);
    printf("  external: %p\n", (void*)&tls_external);
    printf("  public: %p\n", (void*)&tls_public);
    printf("  preserved: %p\n", (void*)&tls_preserved);
    printf("  static: %p\n", (void*)&tls_static);
    printf("  init: %p\n", (void*)&tls_init);
    
    /* Check if we're using emulated TLS */
    #ifndef __HAVE_TLS
    printf("Using emulated TLS\n");
    #endif
    
    return 0;
}
