/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * force the compiler to copy declaration attributes between TLS variables
 * during emulation setup, specifically targeting the uncovered lines in
 * tree-emutls.cc (lines 295-304).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p1, void *p2);
extern int opaque_func(void);
extern void *get_random_ptr(void);

/* Visibility attributes */
#define HIDDEN __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define DEFAULT __attribute__((visibility("default")))

/* Weak symbol attribute */
#define WEAK __attribute__((weak))

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT __attribute__((dllimport))
#endif

/* ================= TLS VARIABLE DECLARATIONS ================= */

/* Public TLS with external linkage and default visibility */
__thread int tls_public_default = 42;
DEFAULT __thread int tls_explicit_default = 100;

/* Weak TLS symbol */
WEAK __thread int tls_weak = 999;

/* Hidden visibility TLS */
HIDDEN __thread int tls_hidden = 123;

/* Protected visibility TLS */
PROTECTED __thread int tls_protected = 456;

/* DLL imported TLS (simulated) */
DLLIMPORT extern __thread int tls_dllimport;

/* Common TLS (tentative definition) */
__thread int tls_common;

/* Static TLS (internal linkage) */
static __thread int tls_static = 789;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 321;

/* Complex initialized TLS */
__thread int tls_complex = 0;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int tls_external;
extern __thread int tls_external_hidden HIDDEN;
extern __thread int tls_external_weak WEAK;

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 555;

/* TLS that might need preservation (address escapes) */
__thread int tls_preserve_me = 777;

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch(idx % 4) {
        case 0: return tls_public_default;
        case 1: return tls_hidden;
        case 2: return tls_static;
        case 3: return tls_volatile;
        default: return 0;
    }
}

/* Function that takes address of TLS variables - forces address computation */
static void take_tls_addresses(void **ptrs, int n) {
    void *addresses[] = {
        (void*)&tls_public_default,
        (void*)&tls_weak,
        (void*)&tls_hidden,
        (void*)&tls_protected,
        (void*)&tls_static,
        (void*)&tls_volatile,
        (void*)&tls_complex,
        (void*)&tls_aligned,
        (void*)&tls_preserve_me,
        (void*)&tls_common
    };
    
    int num_addrs = sizeof(addresses)/sizeof(addresses[0]);
    for (int i = 0; i < n && i < num_addrs; i++) {
        ptrs[i] = addresses[i];
    }
}

/* Function that uses TLS in a loop - may trigger optimizations */
static int process_tls_in_loop(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        tls_static = i;
        sum += tls_public_default;
        sum += inline_tls_access(i);
        
        /* Volatile access prevents dead code elimination */
        sum += tls_volatile;
    }
    return sum;
}

/* Function that conditionally modifies TLS based on runtime values */
static void conditional_tls_modification(int seed) {
    volatile int * volatile ptr_array[10];
    
    /* Take addresses of TLS variables - forces emulation structures */
    take_tls_addresses((void**)ptr_array, 10);
    
    /* Pass TLS addresses to opaque function to prevent optimization */
    for (int i = 0; i < 5; i++) {
        use_ptr(ptr_array[i]);
    }
    
    /* Conditional TLS access based on seed */
    if (seed & 1) {
        tls_hidden = seed;
        use_ptr(&tls_hidden);
    }
    if (seed & 2) {
        tls_protected = seed * 2;
        use_ptr(&tls_protected);
    }
    if (seed & 4) {
        tls_weak = seed * 3;
        use_ptr(&tls_weak);
    }
    
    /* Complex expression with TLS address */
    int *tls_ptr = (seed & 8) ? &tls_public_default : &tls_static;
    *tls_ptr += seed;
    use_ptr(tls_ptr);
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char **argv) {
    int seed = 0;
    
    /* Use command line argument for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = (int)(long)get_random_ptr() % 1000;
    }
    
    /* Initialize complex TLS with runtime value */
    tls_complex = opaque_func() + seed;
    
    /* Initialize TLS common with seed value */
    tls_common = seed;
    
    /* Force preservation of TLS address (simulate asm or external use) */
    void *preserved_tls_addr = &tls_preserve_me;
    use_ptr(preserved_tls_addr);
    
    /* Create a volatile pointer to TLS to prevent optimization */
    volatile int * volatile volatile_tls_ptr = &tls_volatile;
    *volatile_tls_ptr = seed * 2;
    
    /* Process TLS in loops */
    int loop_result = process_tls_in_loop(seed % 100 + 1);
    
    /* Conditional modifications based on seed */
    conditional_tls_modification(seed);
    
    /* Use TLS variables in switch statement */
    int tls_switch_var = 0;
    switch (seed % 5) {
        case 0: tls_switch_var = tls_public_default; break;
        case 1: tls_switch_var = tls_hidden; break;
        case 2: tls_switch_var = tls_protected; break;
        case 3: tls_switch_var = tls_weak; break;
        case 4: tls_switch_var = tls_static; break;
    }
    
    /* Mix TLS variables with array indexing */
    int tls_values[] = {
        tls_public_default,
        tls_hidden,
        tls_protected,
        tls_weak,
        tls_static,
        tls_volatile,
        tls_complex,
        tls_aligned,
        tls_preserve_me,
        tls_common
    };
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    for (int i = 0; i < (int)(sizeof(tls_values)/sizeof(tls_values[0])); i++) {
        checksum ^= tls_values[i];
        checksum = (checksum << 1) | (checksum >> 31); /* Simple mixing */
    }
    
    /* Add loop result and switch variable */
    checksum += loop_result + tls_switch_var;
    
    /* Use checksum in a way that can't be optimized away */
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    /* Final use of TLS addresses with opaque function */
    use_ptr2(&tls_public_default, &tls_hidden);
    use_ptr2(&tls_protected, &tls_weak);
    use_ptr2(&tls_static, &tls_volatile);
    
    return checksum % 256;
}

/* ================= STUB FUNCTIONS FOR LINKING ================= */

/* These would normally be in a separate library, but are stubbed here
   for completeness in a test environment */

void use_ptr(void *p) {
    /* Prevent optimization */
    volatile static int sink;
    sink = (int)(long)p;
}

void use_ptr2(void *p1, void *p2) {
    volatile static int sink;
    sink = (int)(long)p1 + (int)(long)p2;
}

int opaque_func(void) {
    return 42; /* Arbitrary value */
}

void *get_random_ptr(void) {
    static int counter = 0;
    return (void*)(long)(counter++);
}

/* External TLS definitions (simulating another compilation unit) */
__thread int tls_external = 1111;
HIDDEN __thread int tls_external_hidden = 2222;
WEAK __thread int tls_external_weak = 3333;
