/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * specifically exercise the declaration attribute copying logic in
 * tree-emutls.cc (lines 295-304).
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

/* Public TLS with external linkage and default visibility */
__thread int public_tls = 42;
__thread int public_tls_uninit;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 300;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Use dllimport attribute if supported, otherwise just extern */
extern __thread int imported_tls __attribute__((dllimport));
#endif

/* Static TLS (not public) with complex initialization */
static __thread int static_tls = 0;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* TLS with address taken in assembly (hint for DECL_PRESERVE_P) */
__thread int preserved_tls = 700;

/* Common TLS symbol (tentative definition) */
__thread int common_tls;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_tls_with_attr __attribute__((visibility("default")));

/* ============================================
 * FUNCTIONS THAT WORK WITH TLS
 * ============================================ */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    return inline_tls;
}

/* Function that takes address of TLS and uses it in ways that might
 * force emulation structure creation */
static void manipulate_tls_pointers(int seed) {
    /* Array of volatile pointers to prevent optimization */
    volatile void *ptr_array[20];
    int i = 0;
    
    /* Take addresses of various TLS variables */
    ptr_array[i++] = (void*)&public_tls;
    ptr_array[i++] = (void*)&weak_tls;
    ptr_array[i++] = (void*)&hidden_tls;
    ptr_array[i++] = (void*)&protected_tls;
    ptr_array[i++] = (void*)&internal_tls;
    ptr_array[i++] = (void*)&static_tls;
    ptr_array[i++] = (void*)&aligned_tls;
    ptr_array[i++] = (void*)&volatile_tls;
    ptr_array[i++] = (void*)&preserved_tls;
    ptr_array[i++] = (void*)&common_tls;
    
    /* Simulate external function calls with TLS pointers */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)ptr_array[j]);
    }
    
    /* Use inline function that accesses TLS */
    int inline_result = inline_tls_access(seed);
    use_int(inline_result);
    
    /* Complex expression with TLS address */
    int *tls_ptr = &public_tls;
    tls_ptr += (seed & 0x3);  /* Prevent constant propagation */
    use_ptr(tls_ptr);
}

/* Function with dynamic TLS initialization */
static void init_dynamic_tls(void) {
    static __thread int dynamic_tls = 0;
    
    /* Use runtime value for initialization */
    dynamic_tls = get_random_value();
    
    /* Take address and pass to opaque function */
    use_ptr(&dynamic_tls);
    
    /* Use in asm statement to hint preservation */
    __asm__ volatile ("# %0" : : "r"(&dynamic_tls) : "memory");
}

/* Function that mixes TLS and non-TLS data in loops */
static int process_tls_values(int iterations, int seed) {
    int sum = 0;
    
    /* Use seed to make control flow unpredictable */
    if (seed & 1) {
        public_tls += iterations;
    }
    
    if (seed & 2) {
        hidden_tls *= 2;
    }
    
    /* Loop with TLS access */
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS in computation */
        static_tls += i + (seed & 0xFF);
        sum += static_tls;
        
        /* Conditional TLS access */
        if (i % 3 == 0) {
            volatile_tls = i;
            sum += volatile_tls;
        }
        
        /* Call inline function */
        sum += inline_tls_access(i);
    }
    
    /* Another loop with address taking */
    for (int i = 0; i < 5; i++) {
        int *ptr;
        switch (i) {
            case 0: ptr = &public_tls; break;
            case 1: ptr = &weak_tls; break;
            case 2: ptr = &hidden_tls; break;
            case 3: ptr = &protected_tls; break;
            case 4: ptr = &internal_tls; break;
            default: ptr = &common_tls;
        }
        *ptr += i;
        use_ptr(ptr);
    }
    
    return sum;
}

/* ============================================
 * MAIN FUNCTION
 * ============================================ */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize some TLS variables with runtime values */
    public_tls = seed;
    weak_tls = seed * 2;
    hidden_tls = seed * 3;
    protected_tls = seed * 4;
    internal_tls = seed * 5;
    static_tls = seed * 6;
    aligned_tls = seed * 7;
    volatile_tls = seed * 8;
    preserved_tls = seed * 9;
    common_tls = seed * 10;
    
    /* Force address-taking and pointer manipulation */
    manipulate_tls_pointers(seed);
    
    /* Dynamic initialization */
    init_dynamic_tls();
    
    /* Process TLS values with loops and conditionals */
    int iterations = (seed % 100) + 10;  /* Prevent constant propagation */
    int checksum = process_tls_values(iterations, seed);
    
    /* Additional complex TLS usage */
    {
        /* Array of pointers to TLS variables */
        int* tls_ptrs[] = {
            &public_tls, &weak_tls, &hidden_tls, &protected_tls,
            &internal_tls, &static_tls, &aligned_tls, &common_tls
        };
        
        /* Use seed to index unpredictably */
        for (int i = 0; i < 8; i++) {
            int idx = (seed + i) % 8;
            *tls_ptrs[idx] += i;
            checksum += *tls_ptrs[idx];
            
            /* Force address escape */
            use_ptr(tls_ptrs[idx]);
        }
    }
    
    /* Final checksum computation using all TLS variables */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_tls;
    checksum += aligned_tls;
    checksum += volatile_tls;
    checksum += preserved_tls;
    checksum += common_tls;
    
    /* Use inline function one more time */
    checksum += inline_tls_access(checksum);
    
    /* Print result to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    /* Simulate use of external TLS */
    external_tls = checksum % 100;
    use_ptr(&external_tls);
    
    return checksum % 256;
}

/* ============================================
 * EXTERNAL TLS DEFINITIONS
 * (Simulating another compilation unit)
 * ============================================ */

/* In a real multi-file test, these would be in a separate source file */
__thread int external_tls = 999;
__thread int external_tls_with_attr __attribute__((visibility("default"))) = 1111;

/* ============================================
 * STUB FUNCTIONS FOR LINKING
 * (In a real test environment, these would be empty)
 * ============================================ */

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
    /* Return non-constant value */
    static int counter = 0;
    return counter++ + 12345;
}

void side_effect(void) {
    /* Empty but prevents optimization */
    static volatile int dummy = 0;
    dummy++;
}
