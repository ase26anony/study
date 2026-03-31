/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC target("tls")  /* Ensure TLS support is considered */
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Public TLS with explicit visibility and used attribute */
__thread int tls_public_used __attribute__((used, visibility("default"))) = 42;

/* 2. Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 100;

/* 3. Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* 4. Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* 5. External declaration (defined in another file) */
extern __thread int tls_external;

/* 6. DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* 7. Static TLS inside a function context */
static void static_function_context(void) {
    static __thread int tls_static_func = 300;
    tls_static_func++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* 8. TLS with no special attributes */
__thread int tls_plain = 500;

/* ========== FUNCTION DECLARATIONS ========== */
void test_tls_usage(void);
void test_extern_tls(void);
uint32_t compute_tls_checksum(void);

/* ========== HELPER FUNCTIONS ========== */

/* Force variable preservation */
static void preserve_address(void *addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Access all TLS variables to ensure they're used */
void test_tls_usage(void) {
    /* Modify public TLS */
    tls_public_used += 1;
    preserve_address(&tls_public_used);
    
    /* Modify weak TLS */
    if (&tls_weak) {  /* Check if weak symbol is present */
        tls_weak *= 2;
        preserve_address(&tls_weak);
    }
    
    /* Modify common TLS */
    tls_common = 1234;
    preserve_address(&tls_common);
    
    /* Modify hidden TLS */
    tls_hidden -= 5;
    preserve_address(&tls_hidden);
    
    /* Access external TLS */
    tls_external = tls_public_used + 10;
    preserve_address(&tls_external);
    
    /* Modify plain TLS */
    tls_plain = tls_plain * 3 + 1;
    preserve_address(&tls_plain);
    
    /* Call function with static TLS */
    static_function_context();
}

/* Compute checksum of all TLS values */
uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_used;
    sum += tls_common;
    sum += tls_hidden;
    sum += tls_external;
    sum += tls_plain;
    
    if (&tls_weak) {
        sum += tls_weak;
    }
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    printf("Starting emulated TLS attribute test...\n");
    
    /* Initial access to force TLS instantiation */
    test_tls_usage();
    
    /* Multiple accesses to ensure emulation paths are taken */
    for (int i = 0; i < 3; i++) {
        test_tls_usage();
        test_extern_tls();
        
        uint32_t checksum = compute_tls_checksum();
        printf("Iteration %d: TLS checksum = %u\n", i, checksum);
    }
    
    /* Final verification */
    printf("Final TLS values:\n");
    printf("  tls_public_used: %d\n", tls_public_used);
    printf("  tls_common: %d\n", tls_common);
    printf("  tls_hidden: %d\n", tls_hidden);
    printf("  tls_external: %d\n", tls_external);
    printf("  tls_plain: %d\n", tls_plain);
    
    if (&tls_weak) {
        printf("  tls_weak: %d\n", tls_weak);
    }
    
    return 0;
}
