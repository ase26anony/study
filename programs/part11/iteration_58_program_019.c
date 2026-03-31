/* test-emutls-coverage.c
 * Comprehensive test to cover GCC's emulated TLS property copying
 * Lines 295-304 in tree-emutls.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS by targeting older architecture or using flag */
/* Compile with: -O2 -ftls-model=emulated -fno-builtin -m32 (if available) */

/* Prevent dead code elimination */
volatile void *tls_addresses[10];
volatile int tls_values[10];
int checksum = 0;

/* Helper to ensure variables are used */
__attribute__((noinline)) void use_tls_variables(void) {
    /* Take addresses and values of all TLS variables */
    int idx = 0;
    
    /* 1. Weak TLS variable - triggers DECL_WEAK copying */
    extern __thread int tls_weak __attribute__((weak));
    tls_addresses[idx] = &tls_weak;
    tls_values[idx] = tls_weak;
    checksum += (int)(intptr_t)&tls_weak;
    idx++;
    
    /* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
    __thread int tls_hidden __attribute__((visibility("hidden"))) = 42;
    tls_addresses[idx] = &tls_hidden;
    tls_values[idx] = tls_hidden;
    checksum += tls_hidden;
    idx++;
    
    /* 3. Common TLS variable (tentative definition) - triggers DECL_COMMON */
    __thread int tls_common;  /* No initializer = tentative definition */
    tls_common = 100;
    tls_addresses[idx] = &tls_common;
    tls_values[idx] = tls_common;
    checksum += tls_common;
    idx++;
    
    /* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
    extern __thread int tls_external;
    tls_addresses[idx] = &tls_external;
    tls_values[idx] = tls_external;
    checksum += tls_external;
    idx++;
    
    /* 5. Public TLS with used attribute - influences TREE_PUBLIC and DECL_PRESERVE_P */
    __thread int tls_public __attribute__((used)) = 200;
    tls_addresses[idx] = &tls_public;
    tls_values[idx] = tls_public;
    checksum += tls_public;
    idx++;
    
    /* 6. TLS in function scope - affects DECL_CONTEXT */
    static __thread int tls_static_local = 300;
    tls_addresses[idx] = &tls_static_local;
    tls_values[idx] = tls_static_local;
    checksum += tls_static_local;
    idx++;
    
    /* 7. TLS with preserved attribute (via retain or noinline function) */
    /* Using in noinline function should help with DECL_PRESERVE_P */
    __thread int tls_preserved = 400;
    tls_addresses[idx] = &tls_preserved;
    tls_values[idx] = tls_preserved;
    checksum += tls_preserved;
    idx++;
    
#ifdef _WIN32
    /* 8. DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
    /* Note: This requires actual DLL import setup to fully test */
    __declspec(dllimport) __thread int tls_imported;
    tls_addresses[idx] = &tls_imported;
    tls_values[idx] = tls_imported;
    checksum += tls_imported;
    idx++;
#endif
    
    /* Force all TLS variables to be referenced */
    if (&tls_weak != &tls_hidden) {
        /* This comparison forces both addresses to be taken */
        checksum += 1;
    }
}

/* Another function that uses TLS to ensure they're preserved */
__attribute__((noinline, used)) void modify_tls(void) {
    __thread int tls_preserved = 500;
    tls_preserved++;
    
    static __thread int tls_static_local = 600;
    tls_static_local += 2;
}

/* Weak TLS definition (provide one in case no strong definition exists) */
__thread int tls_weak = 10;

/* External TLS definition would normally be in another file */
__thread int tls_external = 30;

int main(void) {
    /* Initialize checksum */
    checksum = 0;
    
    /* Use TLS variables in non-inlineable function */
    use_tls_variables();
    
    /* Modify TLS in another function */
    modify_tls();
    
    /* Print something based on TLS usage to prevent optimization */
    printf("TLS coverage test - checksum: %d\n", checksum);
    
    /* Print addresses to ensure TLS variables are instantiated */
    printf("TLS addresses differ: %p vs %p\n", 
           (void*)&tls_weak, (void*)&tls_hidden);
    
    return 0;
}
