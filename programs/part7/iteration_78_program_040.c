/* File1.c - Defines TLS variables with various attributes */

/* Force emulated TLS even if native is available */
#pragma GCC tls_model emulated

#include <stdio.h>

/* 1. Public TLS with explicit visibility and used attribute */
__thread int tls_public __attribute__((used, visibility("default"))) = 42;

/* 2. Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* 3. Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* 5. External declaration (defined in File2.c) */
extern __thread int tls_external;

/* 6. DLL import simulation */
#ifdef _WIN32
__thread int tls_dllimport __attribute__((dllimport));
#else
/* Simulate with weak external on non-Windows */
extern __thread int tls_dllimport __attribute__((weak));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 300;
    tls_static_func++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* 8. TLS with all attributes combined */
__thread int tls_combo __attribute__((used, weak, visibility("hidden"))) = 400;

/* Function to use TLS variables */
void use_tls_variables(void) {
    /* Access all TLS variables to ensure they're used */
    tls_public++;
    tls_hidden += 2;
    
    if (tls_weak > 0) {
        tls_weak--;
    }
    
    tls_common = tls_public + tls_hidden;
    
    /* External TLS access */
    if (&tls_external != NULL) {
        tls_external++;
    }
    
    /* DLL import simulation access */
    if (&tls_dllimport != NULL) {
        /* Cast to volatile to prevent optimization */
        *(volatile int*)&tls_dllimport = 999;
    }
    
    tls_combo *= 2;
    
    /* Call function with static TLS */
    func_with_static_tls();
    
    /* Force address taking for all variables */
    asm volatile("" : : 
        "r"(&tls_public), "r"(&tls_hidden), "r"(&tls_weak),
        "r"(&tls_common), "r"(&tls_external), "r"(&tls_dllimport),
        "r"(&tls_combo));
}

/* Checksum function for TLS values */
int calculate_tls_checksum(void) {
    int sum = 0;
    
    sum += tls_public;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    
    /* Only add external if defined */
    if (&tls_external != NULL) {
        sum += tls_external;
    }
    
    if (&tls_dllimport != NULL) {
        sum += tls_dllimport;
    }
    
    sum += tls_combo;
    
    return sum;
}
