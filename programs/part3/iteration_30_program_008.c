/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios in GCC,
 * specifically targeting the declaration attribute copying logic in
 * tree-emutls.cc lines 295-304.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p);
extern int opaque_func(void);
extern void *get_external_ptr(void);

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

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

/* DLL import simulation (for Windows-targeting compilations) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Use dllimport-like attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Static TLS with preservation requirements */
static __thread int static_preserved_tls = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* Dynamic initializer TLS */
extern int get_random(void);
__thread int dynamic_init_tls = 0; /* Will be initialized in main */

/* Alignment-specified TLS */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_hidden_tls __attribute__((visibility("hidden")));

/* ================= FUNCTIONS THAT OPERATE ON TLS ================= */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch (idx % 4) {
        case 0: return public_tls;
        case 1: return hidden_tls;
        case 2: return protected_tls;
        case 3: return internal_tls;
        default: return 0;
    }
}

/* Function that takes address of TLS and uses it in complex ways */
static void manipulate_tls_pointers(int seed) {
    /* Array of volatile pointers to prevent optimization */
    volatile void *ptr_array[12];
    int i = 0;
    
    /* Take addresses of various TLS variables */
    ptr_array[i++] = (void*)&public_tls;
    ptr_array[i++] = (void*)&weak_tls;
    ptr_array[i++] = (void*)&hidden_tls;
    ptr_array[i++] = (void*)&protected_tls;
    ptr_array[i++] = (void*)&internal_tls;
    ptr_array[i++] = (void*)&static_preserved_tls;
    ptr_array[i++] = (void*)&volatile_tls;
    ptr_array[i++] = (void*)&aligned_tls;
    ptr_array[i++] = (void*)&common_tls;
    
    /* Use seed to make access pattern unpredictable */
    if (seed & 1) {
        ptr_array[i++] = (void*)&external_tls;
    }
    if (seed & 2) {
        ptr_array[i++] = (void*)&external_hidden_tls;
    }
    
    /* Pass pointers to opaque functions */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)ptr_array[j]);
    }
    
    /* Complex expression with TLS address arithmetic */
    int *tls_ptr = &public_tls;
    for (int j = 0; j < (seed & 0x3); j++) {
        tls_ptr++;
        use_ptr2(tls_ptr);
    }
}

/* Function with TLS in loop transformations */
static void tls_loop_operations(int iterations, int seed) {
    /* Use TLS as loop counter or accumulator */
    static __thread int loop_tls = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        loop_tls += inline_tls_access(i + seed);
        public_tls ^= (i * seed);
        
        /* Conditional TLS access */
        if (i % 3 == 0) {
            hidden_tls += loop_tls;
        } else if (i % 3 == 1) {
            protected_tls -= public_tls;
        } else {
            internal_tls *= (seed % 10) + 1;
        }
        
        /* Address taking within loop */
        volatile int *addr = &loop_tls;
        *addr += i;
    }
}

/* Function that escapes TLS addresses */
static void escape_tls_addresses(void) {
    /* Store TLS addresses in global-like locations */
    static void *escaped_addrs[5];
    static int escape_idx = 0;
    
    escaped_addrs[escape_idx++ % 5] = &public_tls;
    escaped_addrs[escape_idx++ % 5] = &hidden_tls;
    escaped_addrs[escape_idx++ % 5] = &protected_tls;
    escaped_addrs[escape_idx++ % 5] = &static_preserved_tls;
    
    /* Use asm to force preservation */
    __asm__ volatile ("" : : "r"(&public_tls), "r"(&hidden_tls));
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize TLS with dynamic values */
    dynamic_init_tls = seed * 2;
    common_tls = seed % 100;
    
    /* Simulate external TLS definitions */
    external_tls = seed + 1000;
    external_hidden_tls = seed + 2000;
    
    /* Force TLS emulation through various operations */
    manipulate_tls_pointers(seed);
    
    /* Complex loop with TLS */
    tls_loop_operations(50 + (seed % 50), seed);
    
    /* Escape addresses to force preservation */
    escape_tls_addresses();
    
    /* Use volatile accesses to prevent optimization */
    volatile int *volatile_ptr = &volatile_tls;
    for (int i = 0; i < 10; i++) {
        *volatile_ptr += inline_tls_access(i);
    }
    
    /* Compute checksum of all TLS values to prevent dead code elimination */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_preserved_tls;
    checksum += volatile_tls;
    checksum += dynamic_init_tls;
    checksum += aligned_tls;
    checksum += common_tls;
    checksum += external_tls;
    checksum += external_hidden_tls;
    
    /* Mix in opaque function result */
    checksum += opaque_func();
    
    /* Use checksum so it can't be optimized away */
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    /* Final use of all TLS addresses */
    void *all_addrs[] = {
        &public_tls, &weak_tls, &hidden_tls, &protected_tls,
        &internal_tls, &static_preserved_tls, &volatile_tls,
        &dynamic_init_tls, &aligned_tls, &common_tls,
        &external_tls, &external_hidden_tls
    };
    
    for (size_t i = 0; i < sizeof(all_addrs)/sizeof(all_addrs[0]); i++) {
        use_ptr(all_addrs[i]);
    }
    
    return checksum % 256;
}

/* ================= STUB FUNCTIONS FOR COMPILATION ================= */

/* These would be provided in a separate file in a real test setup */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_ptr2(void *p) {
    /* Another opaque function */
    (void)p;
}

int opaque_func(void) {
    return 42;
}

/* External TLS definitions */
__thread int external_tls = 9999;
__thread int external_hidden_tls __attribute__((visibility("hidden"))) = 8888;
