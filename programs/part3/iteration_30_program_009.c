/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * force the compiler to copy declaration attributes between TLS variables
 * during emulation setup, specifically targeting the uncovered lines
 * in tree-emutls.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int i);
extern int get_random_value(void);
extern void side_effect(void);

/* ============================================
 * TLS VARIABLES WITH DIVERSE ATTRIBUTES
 * ============================================ */

/* Public TLS with external linkage, default visibility */
__thread int public_tls = 42;
extern __thread int external_public_tls;  /* Will be defined later */

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Use dllimport attribute if supported, otherwise simulate */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;  /* No initializer -> common symbol */

/* Static TLS (file scope, internal linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* Dynamic initializer TLS */
__thread int dynamic_tls = 0;  /* Will be initialized in main */

/* Alignment-specified TLS */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* ============================================
 * EXTERNAL TLS DEFINITIONS
 * ============================================ */

/* Define the external TLS variable (simulating another compilation unit) */
__thread int external_public_tls = 999;

/* ============================================
 * FUNCTIONS THAT USE TLS
 * ============================================ */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch (idx % 4) {
        case 0: return public_tls;
        case 1: return static_tls;
        case 2: return hidden_tls;
        case 3: return volatile_tls;
        default: return 0;
    }
}

/* Function that takes address of TLS variables */
static void take_tls_addresses(void) {
    void *addresses[12];
    volatile void *volatile_addresses[12];  /* volatile to prevent optimization */
    
    /* Take addresses of various TLS variables */
    addresses[0] = &public_tls;
    addresses[1] = &external_public_tls;
    addresses[2] = &weak_tls;
    addresses[3] = &hidden_tls;
    addresses[4] = &internal_tls;
    addresses[5] = &protected_tls;
    addresses[6] = &dllimport_tls;
    addresses[7] = &common_tls;
    addresses[8] = &static_tls;
    addresses[9] = &volatile_tls;
    addresses[10] = &dynamic_tls;
    addresses[11] = &aligned_tls;
    
    /* Copy to volatile array */
    for (int i = 0; i < 12; i++) {
        volatile_addresses[i] = addresses[i];
    }
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < 12; i++) {
        use_ptr((void *)volatile_addresses[i]);
    }
}

/* Complex expression with TLS address computation */
static int compute_with_tls(int seed) {
    int result = 0;
    
    /* Use TLS variables in complex expressions */
    result += public_tls * seed;
    result -= static_tls / (seed + 1);
    result ^= hidden_tls;
    
    /* Take address and dereference */
    int *ptr = &volatile_tls;
    result += *ptr;
    
    /* Use inline function */
    result += inline_tls_access(seed);
    
    /* Chain of TLS accesses */
    result = result + weak_tls - internal_tls + protected_tls;
    
    return result;
}

/* Function that might cause TLS declaration cloning */
static void tls_in_loop(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses in loop */
        sum += public_tls + i;
        sum -= static_tls * (i % 3);
        
        /* Conditional TLS access */
        if (i % 2 == 0) {
            sum += hidden_tls;
        } else {
            sum += volatile_tls;
        }
        
        /* Inline function call inside loop */
        sum += inline_tls_access(i);
        
        /* Address taking inside loop */
        if (i % 5 == 0) {
            void *addr = &aligned_tls;
            use_ptr(addr);
        }
    }
    
    use_int(sum);
}

/* ============================================
 * MAIN FUNCTION
 * ============================================ */

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    srand(seed);
    
    /* Dynamic initialization of TLS */
    dynamic_tls = get_random_value();
    
    /* Initialize common TLS */
    common_tls = seed * 2;
    
    /* Take addresses of all TLS variables */
    take_tls_addresses();
    
    /* Complex computations with TLS */
    int result1 = compute_with_tls(seed);
    int result2 = compute_with_tls(seed + 1);
    
    /* Use TLS in loops with varying iterations */
    tls_in_loop(seed % 100 + 50);
    
    /* Array of TLS pointers for indirect access */
    int *tls_ptrs[] = {
        &public_tls,
        &static_tls,
        &hidden_tls,
        &volatile_tls,
        &weak_tls,
        &dynamic_tls,
        &common_tls,
        &aligned_tls
    };
    
    /* Unpredictable access pattern */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        int idx = (seed + i) % (sizeof(tls_ptrs) / sizeof(tls_ptrs[0]));
        checksum += *tls_ptrs[idx];
        checksum ^= i;
        
        /* Occasionally take address and pass to opaque function */
        if (i % 7 == 0) {
            use_ptr(tls_ptrs[idx]);
        }
    }
    
    /* Force use of all TLS variables in final computation */
    checksum += public_tls;
    checksum += external_public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += internal_tls;
    checksum += protected_tls;
    checksum += dllimport_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += aligned_tls;
    
    /* Mix with inline function results */
    for (int i = 0; i < 10; i++) {
        checksum += inline_tls_access(seed + i);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional side effect */
    side_effect();
    
    return checksum % 256;
}

/* ============================================
 * STUB FUNCTIONS (would be in separate file in real test)
 * ============================================ */

/* These are defined here just to make the program compilable.
 * In a real coverage test, they would be empty or in a separate file.
 */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_int(int i) {
    /* Prevent optimization */
    static volatile int last_int = 0;
    last_int = i;
}

int get_random_value(void) {
    return rand();
}

void side_effect(void) {
    /* Do nothing but prevent optimization */
    static volatile int counter = 0;
    counter++;
}
