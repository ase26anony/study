/* Main file with various TLS declarations to trigger attribute copying */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")  /* Prevent optimization removing declarations */
#endif

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Public TLS with explicit visibility and used attribute */
__thread int tls_public_used __attribute__((used, visibility("default"))) = 42;

/* 2. Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 100;

/* 3. Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* 4. Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* 5. DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with a custom attribute if not on Windows */
#define DLLIMPORT __attribute__((dllimport))
DLLIMPORT __thread int tls_dllimport;
#endif

/* 6. External TLS declaration (defined in another file) */
extern __thread int tls_external;

/* 7. Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 300;
    tls_func_static++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_func_static));
}

/* 8. TLS with all attributes combined */
__thread int tls_complete __attribute__((used, weak, visibility("hidden"))) = 400;

/* ===== FUNCTION DECLARATIONS ===== */
void test_tls_access(void);
void test_external_tls(void);
uint32_t compute_tls_checksum(void);

/* ===== TEST FUNCTIONS ===== */

/* Function that uses all TLS variables to prevent elimination */
void use_all_tls(void) {
    /* Access and modify each TLS variable */
    tls_public_used += 1;
    tls_weak += 2;
    tls_common += 3;
    tls_hidden += 4;
    /* tls_dllimport accessed elsewhere */
    tls_external += 6;  /* Defined in another file */
    tls_complete += 7;
    
    /* Call function with static TLS */
    func_with_static_tls();
    
    /* Force compiler to keep all TLS variables */
    asm volatile("" : : 
        "r"(&tls_public_used),
        "r"(&tls_weak),
        "r"(&tls_common),
        "r"(&tls_hidden),
        "r"(&tls_complete)
    );
}

/* Compute checksum of all TLS values */
uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_used;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    /* tls_dllimport might be 0 if not linked */
    sum += tls_external;
    sum += tls_complete;
    
    return sum;
}

int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initialize TLS variables */
    tls_common = 50;  /* Initialize the common TLS */
    
    /* Use all TLS variables multiple times */
    for (int i = 0; i < 3; i++) {
        use_all_tls();
        test_tls_access();
        test_external_tls();
    }
    
    /* Compute and print checksum */
    uint32_t checksum = compute_tls_checksum();
    printf("TLS checksum: %u\n", checksum);
    
    /* Print individual values for verification */
    printf("tls_public_used: %d\n", tls_public_used);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_common: %d\n", tls_common);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_external: %d\n", tls_external);
    printf("tls_complete: %d\n", tls_complete);
    
    return 0;
}
