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
extern int get_random_value(void);
extern void escape_to_heap(void **ptr);

/* ============================================
 * TLS VARIABLES WITH DIVERSE ATTRIBUTES
 * ============================================ */

/* Public TLS with external linkage */
__thread int public_tls = 42;
__thread int public_tls_uninit;

/* Weak TLS symbol */
__attribute__((weak)) __thread int weak_tls = 100;

/* Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int hidden_tls = 200;

/* Protected visibility TLS */
__attribute__((visibility("protected"))) __thread int protected_tls = 300;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Use dllimport-like attribute if supported, else regular TLS */
# ifdef __has_attribute
#  if __has_attribute(dllimport)
__attribute__((dllimport)) __thread int dllimport_tls;
#  else
__thread int dllimport_tls = 400;
#  endif
# else
__thread int dllimport_tls = 400;
# endif
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Static TLS (internal linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* TLS with dynamic initialization */
__thread int dynamic_tls = 0;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* ============================================
 * HELPER FUNCTIONS
 * ============================================ */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch (idx % 4) {
        case 0: return public_tls;
        case 1: return static_tls;
        case 2: return hidden_tls;
        case 3: return protected_tls;
        default: return 0;
    }
}

/* Function that takes address of TLS variables */
static void take_tls_addresses(void **ptrs, int n) {
    /* Store addresses of various TLS variables */
    ptrs[0] = (void *)&public_tls;
    ptrs[1] = (void *)&weak_tls;
    ptrs[2] = (void *)&hidden_tls;
    ptrs[3] = (void *)&protected_tls;
    ptrs[4] = (void *)&dllimport_tls;
    ptrs[5] = (void *)&common_tls;
    ptrs[6] = (void *)&static_tls;
    ptrs[7] = (void *)&volatile_tls;
    ptrs[8] = (void *)&aligned_tls;
    
    /* Force compiler to consider these addresses as escaping */
    for (int i = 0; i < n && i < 9; i++) {
        use_ptr(ptrs[i]);
    }
}

/* Function that uses TLS in a loop - may trigger optimizations */
static int process_tls_values(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        sum += public_tls;
        sum += inline_tls_access(i);
        
        /* Modify TLS variables */
        static_tls += i;
        hidden_tls -= i % 3;
        
        /* Use volatile TLS */
        sum += volatile_tls;
        
        /* Take address in loop */
        void *addr = (void *)&protected_tls;
        use_ptr(addr);
    }
    
    return sum;
}

/* ============================================
 * MAIN FUNCTION
 * ============================================ */

int main(int argc, char **argv) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    srand(seed);
    
    /* Initialize dynamic TLS with a function call */
    dynamic_tls = get_random_value();
    
    /* Array to store TLS addresses */
    void *tls_addresses[20];
    
    /* Take addresses of TLS variables */
    take_tls_addresses(tls_addresses, 20);
    
    /* Escape some addresses to heap simulation */
    escape_to_heap(tls_addresses);
    
    /* Complex expression with TLS address-taking */
    void *complex_ptr = (void *)(&public_tls + (&weak_tls - &public_tls));
    use_ptr(complex_ptr);
    
    /* Use TLS variables in conditionals */
    int mode = seed % 4;
    int result = 0;
    
    switch (mode) {
        case 0:
            result = public_tls + weak_tls;
            break;
        case 1:
            result = hidden_tls - protected_tls;
            break;
        case 2:
            result = static_tls * volatile_tls;
            break;
        case 3:
            result = aligned_tls / (public_tls + 1);
            break;
    }
    
    /* Process TLS values with loops */
    int loop_count = 10 + (seed % 100);
    int loop_result = process_tls_values(loop_count);
    
    /* Access external TLS variables */
    external_tls = seed;
    result += external_tls;
    
    if (&external_weak_tls != NULL) {
        external_weak_tls = seed * 2;
        result += external_weak_tls;
    }
    
    /* Mix with runtime values to prevent optimization */
    int runtime_value;
    if (argc > 2) {
        runtime_value = atoi(argv[2]);
    } else {
        runtime_value = 777;
    }
    
    /* Index into TLS variables using runtime value */
    int *tls_array[] = {
        &public_tls, &weak_tls, &hidden_tls, &protected_tls,
        &static_tls, &volatile_tls, &aligned_tls
    };
    
    int tls_index = runtime_value % 7;
    result += *tls_array[tls_index];
    
    /* Compute checksum of all TLS values */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += dllimport_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += aligned_tls;
    checksum += external_tls;
    
    /* Use checksum so it can't be optimized away */
    printf("TLS checksum: %d, Result: %d, Loop result: %d\n", 
           checksum, result, loop_result);
    
    return (checksum % 256);
}

/* ============================================
 * EXTERNAL FUNCTION STUBS (for linking)
 * ============================================ */

/* These would be defined elsewhere in a real test environment */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_ptr2(void *p1, void *p2) {
    static volatile void *last_p1 = NULL;
    static volatile void *last_p2 = NULL;
    last_p1 = p1;
    last_p2 = p2;
}

int get_random_value(void) {
    return rand() % 1000;
}

void escape_to_heap(void **ptr) {
    static void *escaped_ptr = NULL;
    escaped_ptr = ptr;
}

/* External TLS definitions (simulating another compilation unit) */
__thread int external_tls = 999;
__attribute__((weak)) __thread int external_weak_tls = 888;
