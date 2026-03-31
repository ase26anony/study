/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and force
 * the compiler to copy declaration attributes between TLS variables.
 * It uses various TLS variables with different linkage, visibility,
 * and storage attributes to cover the uncovered lines in tree-emutls.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int opaque_func(void);
extern void escape_ptr(void **p);

/* Global seed for unpredictable control flow */
static int global_seed = 0;

/* ===================== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===================== */

/* Public TLS with external linkage - will be TREE_PUBLIC */
__thread int public_tls = 42;
__thread int public_tls_uninit;

/* Weak TLS symbol - will be DECL_WEAK */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS - tests DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Use dllimport attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Static TLS (file scope) - not TREE_PUBLIC */
static __thread int static_tls = 500;

/* TLS with alignment requirement - might trigger special handling */
__thread int aligned_tls __attribute__((aligned(64))) = 600;

/* TLS with volatile qualification for preventing optimization */
volatile __thread int volatile_tls = 700;

/* TLS with dynamic initialization */
extern int get_random(void);
__thread int dynamic_tls = 0; /* Will be initialized in main */

/* TLS common symbol (tentative definition) - tests DECL_COMMON */
__thread int common_tls; /* No initializer */

/* External TLS declaration (simulating another compilation unit) */
extern __thread int external_tls;

/* TLS that might need preservation (used in asm-like context) */
__thread int preserve_tls = 800;

/* ===================== FUNCTIONS THAT USE TLS ===================== */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch(idx % 5) {
        case 0: return public_tls;
        case 1: return hidden_tls;
        case 2: return static_tls;
        case 3: return aligned_tls;
        case 4: return volatile_tls;
        default: return 0;
    }
}

/* Function that takes address of TLS and escapes it */
static void take_tls_addresses(void) {
    void *tls_ptrs[15];
    int i = 0;
    
    /* Take addresses of all TLS variables */
    tls_ptrs[i++] = &public_tls;
    tls_ptrs[i++] = &public_tls_uninit;
    tls_ptrs[i++] = &weak_tls;
    tls_ptrs[i++] = &hidden_tls;
    tls_ptrs[i++] = &internal_tls;
    tls_ptrs[i++] = &protected_tls;
    tls_ptrs[i++] = &dllimport_tls;
    tls_ptrs[i++] = &static_tls;
    tls_ptrs[i++] = &aligned_tls;
    tls_ptrs[i++] = &volatile_tls;
    tls_ptrs[i++] = &dynamic_tls;
    tls_ptrs[i++] = &common_tls;
    tls_ptrs[i++] = &preserve_tls;
    
    /* Escape pointers to prevent optimization */
    for (int j = 0; j < i; j++) {
        use_ptr(tls_ptrs[j]);
    }
    
    /* Additional escape through global function */
    escape_ptr(&tls_ptrs[0]);
}

/* Function with complex TLS usage pattern */
static int compute_with_tls(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        int val = inline_tls_access(i + global_seed);
        
        /* Modify TLS variables */
        public_tls += i;
        hidden_tls ^= val;
        static_tls *= (i % 3) + 1;
        
        /* Use volatile TLS */
        sum += volatile_tls;
        
        /* Conditional TLS access */
        if (i % 2 == 0) {
            aligned_tls -= val;
        } else {
            protected_tls += val;
        }
        
        /* Address computation involving TLS */
        int *ptr = &public_tls;
        ptr[i % 2] = i;
    }
    
    return sum;
}

/* Function that simulates asm-like usage for DECL_PRESERVE_P */
static void asm_like_tls_usage(void) {
    /* Simulate asm statement that might preserve TLS */
    int local;
    
    /* Pattern that looks like it might be used in asm */
    local = preserve_tls;
    preserve_tls = local + 1;
    
    /* Take address and do pointer arithmetic */
    int *ptr = &preserve_tls;
    for (int i = 0; i < 3; i++) {
        ptr[i] = i * 100;
    }
}

/* ===================== MAIN FUNCTION ===================== */

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        global_seed = atoi(argv[1]) % 100;
    } else {
        global_seed = 42;
    }
    
    /* Dynamic initialization of TLS */
    dynamic_tls = global_seed * 10;
    
    /* Initialize common TLS */
    common_tls = global_seed * 20;
    
    /* Simulate external TLS access */
    extern __thread int external_tls;
    external_tls = global_seed * 30;
    
    /* Array of volatile pointers to TLS to prevent optimization */
    volatile int *volatile_tls_ptrs[10];
    volatile_tls_ptrs[0] = &public_tls;
    volatile_tls_ptrs[1] = &hidden_tls;
    volatile_tls_ptrs[2] = &static_tls;
    volatile_tls_ptrs[3] = &volatile_tls;
    volatile_tls_ptrs[4] = &aligned_tls;
    volatile_tls_ptrs[5] = &protected_tls;
    volatile_tls_ptrs[6] = &internal_tls;
    volatile_tls_ptrs[7] = &dynamic_tls;
    volatile_tls_ptrs[8] = &common_tls;
    volatile_tls_ptrs[9] = &preserve_tls;
    
    /* Unpredictable access pattern */
    int iterations = 50 + (global_seed % 50);
    
    /* Phase 1: Take addresses and escape them */
    take_tls_addresses();
    
    /* Phase 2: Complex computation with TLS */
    int sum1 = compute_with_tls(iterations);
    
    /* Phase 3: Asm-like usage */
    asm_like_tls_usage();
    
    /* Phase 4: Volatile accesses through array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 3; j++) {
            *volatile_tls_ptrs[i] += j + i;
        }
    }
    
    /* Phase 5: Mix with runtime values */
    int runtime_mod = 0;
    if (argc > 2) {
        runtime_mod = atoi(argv[2]);
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Unpredictable index */
        int idx = (i * 17 + runtime_mod) % 10;
        *volatile_tls_ptrs[idx] += inline_tls_access(i);
    }
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    checksum += public_tls;
    checksum += public_tls_uninit;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += internal_tls;
    checksum += protected_tls;
    checksum += dllimport_tls;
    checksum += static_tls;
    checksum += aligned_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += common_tls;
    checksum += preserve_tls;
    checksum += external_tls;
    checksum += sum1;
    
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 100;
}

/* ===================== EXTERNAL FUNCTION STUBS ===================== */

/* These would be defined elsewhere in a real test environment */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void escape_ptr(void **p) {
    /* Simulate pointer escape */
    static void *escaped_ptr = NULL;
    escaped_ptr = *p;
}

int get_random(void) {
    return rand();
}
