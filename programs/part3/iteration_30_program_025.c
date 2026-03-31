/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * force the compiler to copy declaration attributes between TLS variables
 * during emulation setup, specifically targeting the uncovered lines
 * in tree-emutls.cc (lines 295-304).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p);
extern int get_random_value(void);
extern void side_effect(void);

/* Force TLS emulation by using __thread extensively with varied attributes */

/* Public TLS with external linkage */
__thread int public_tls = 42;
__thread int public_tls_uninit;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with attribute if supported */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Static TLS with preservation requirement */
static __thread int static_preserved_tls = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* TLS with dynamic initialization */
__thread int dynamic_tls = 0;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* Inline function that accesses TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    
    /* Take address to force TLS machinery */
    void *addr = &inline_tls;
    use_ptr(addr);
    
    return inline_tls;
}

/* Another function that takes address of TLS variables */
static void process_tls_pointers(int seed) {
    /* Array of volatile pointers to prevent optimization */
    volatile void *ptrs[20];
    int i = 0;
    
    /* Take addresses of various TLS variables */
    ptrs[i++] = (void*)&public_tls;
    ptrs[i++] = (void*)&weak_tls;
    ptrs[i++] = (void*)&hidden_tls;
    ptrs[i++] = (void*)&internal_tls;
    ptrs[i++] = (void*)&protected_tls;
    ptrs[i++] = (void*)&imported_tls;
    ptrs[i++] = (void*)&common_tls;
    ptrs[i++] = (void*)&static_preserved_tls;
    ptrs[i++] = (void*)&volatile_tls;
    ptrs[i++] = (void*)&dynamic_tls;
    ptrs[i++] = (void*)&aligned_tls;
    
    /* Use external TLS */
    ptrs[i++] = (void*)&external_tls;
    ptrs[i++] = (void*)&external_weak_tls;
    
    /* Pass pointers to opaque function to prevent optimization */
    for (int j = 0; j < i; j++) {
        use_ptr2((void*)ptrs[j]);
    }
    
    /* Mix with runtime value to prevent dead code elimination */
    if (seed & 1) {
        public_tls += seed;
    }
    if (seed & 2) {
        weak_tls -= seed;
    }
}

/* Function that creates complex TLS usage pattern */
static void complex_tls_operations(int iterations) {
    /* Local TLS inside function */
    static __thread int local_func_tls = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Access multiple TLS variables in loop */
        public_tls += i;
        hidden_tls ^= public_tls;
        internal_tls = internal_tls * 2 + 1;
        
        /* Use inline function with TLS */
        int inline_result = inline_tls_access(i);
        protected_tls += inline_result;
        
        /* Take address within loop */
        void *addr1 = &local_func_tls;
        void *addr2 = &volatile_tls;
        use_ptr(addr1);
        use_ptr(addr2);
        
        /* Conditional TLS access */
        if (i % 3 == 0) {
            volatile_tls = weak_tls;
        } else if (i % 3 == 1) {
            aligned_tls = hidden_tls;
        } else {
            common_tls = internal_tls;
        }
        
        /* Dynamic initialization simulation */
        dynamic_tls = get_random_value();
    }
    
    /* Force preservation by using address in asm (simulated) */
    __asm__ volatile ("# TLS address: %0" : : "r"(&local_func_tls));
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for runtime control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize dynamic TLS with runtime value */
    dynamic_tls = seed;
    
    /* Initialize external TLS (simulating definition) */
    external_tls = seed * 2;
    external_weak_tls = seed * 3;
    
    /* Process TLS pointers with varied attributes */
    process_tls_pointers(seed);
    
    /* Perform complex operations based on seed */
    int iterations = (seed % 10) + 5;
    complex_tls_operations(iterations);
    
    /* Use TLS variables in switch statement */
    switch (seed % 4) {
        case 0:
            public_tls += hidden_tls;
            break;
        case 1:
            weak_tls += internal_tls;
            break;
        case 2:
            protected_tls += aligned_tls;
            break;
        case 3:
            common_tls += volatile_tls;
            break;
    }
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    checksum += public_tls;
    checksum += public_tls_uninit;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += internal_tls;
    checksum += protected_tls;
    checksum += imported_tls;
    checksum += common_tls;
    checksum += static_preserved_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += aligned_tls;
    checksum += external_tls;
    checksum += external_weak_tls;
    
    /* Use inline function one more time */
    checksum += inline_tls_access(seed);
    
    /* Create side effect with checksum */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional forced usage to ensure all TLS variables are referenced */
    side_effect();
    
    return checksum & 0xFF;
}

/* Stub definitions for opaque functions (for actual compilation) */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_ptr2(void *p) {
    /* Another opaque use */
    (void)p;
}

int get_random_value(void) {
    /* Not truly random, but unpredictable to compiler */
    static int counter = 0;
    return ++counter;
}

void side_effect(void) {
    /* Force compiler to keep all preceding operations */
    static int dummy = 0;
    dummy++;
}
