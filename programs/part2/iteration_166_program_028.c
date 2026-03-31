/*
 * test_target.c - Comprehensive test to trigger GCC driver CPU cache detection
 * 
 * This program uses multiple techniques to force the GCC driver to parse
 * Intel CPUID cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.) during compilation.
 * The uncovered switch-case block in driver-i386.cc (lines 127-244) should be
 * exercised when the driver detects CPU cache parameters.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* ============================================
   PATTERN 1: Function Multiversioning with Target Attributes
   ============================================ */

/* Core function with multiple architecture-specific implementations */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void cache_sensitive_computation(int* data, int size) {
    /* Simple computation that benefits from cache awareness */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * (i % 256);
    }
    data[0] = sum;
}

/* Individual target-specific functions to ensure driver processes each */
__attribute__((target("arch=core2")))
void core2_optimized(int* data, int size) {
    /* Use prefetch hints sized for Core2 cache (typically 64-byte lines) */
    for (int i = 0; i < size; i += 16) { /* 16 * 4 bytes = 64 bytes */
        __builtin_prefetch(&data[i + 16], 0, 3);
        data[i] = data[i] * 2 + 1;
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized(int* data, int size) {
    /* Nehalem has improved prefetch, use different stride */
    for (int i = 0; i < size; i += 8) { /* 8 * 4 bytes = 32 bytes */
        __builtin_prefetch(&data[i + 32], 0, 2);
        data[i] = data[i] * 3 - 2;
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized(int* data, int size) {
    /* Sandy Bridge AVX optimization hint */
    for (int i = 0; i < size; i += 32) { /* 32 * 4 bytes = 128 bytes */
        __builtin_prefetch(&data[i + 64], 0, 1);
        data[i] = data[i] * 4 + 3;
    }
}

/* ============================================
   PATTERN 2: ifunc for Runtime Dispatch
   ============================================ */

typedef void (*compute_func_t)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] << 1) | 0x1;
    }
}

static void compute_avx(int* data, int size) {
    /* Simulate AVX-optimized path */
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] << 2) | 0x3;
    }
}

static compute_func_t resolver(void) {
    /* This forces CPU detection during ifunc resolution */
    if (__builtin_cpu_supports("avx2")) {
        return compute_avx;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_avx;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_default;
    }
    return compute_default;
}

void compute_dynamic(int* data, int size) 
    __attribute__((ifunc("resolver")));

/* ============================================
   PATTERN 3: Direct CPUID Queries
   ============================================ */

static void cpuid_query(uint32_t leaf, uint32_t* eax, uint32_t* ebx, 
                        uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(0)
    );
}

/* Constructor runs before main, forcing early CPU detection */
__attribute__((constructor))
static void early_cpu_detection(void) {
    printf("=== Early CPU Detection (Constructor) ===\n");
    
    /* Initialize CPU builtins */
    __builtin_cpu_init();
    
    /* Query CPUID leaf 2 (Cache Descriptors) */
    uint32_t eax, ebx, ecx, edx;
    cpuid_query(2, &eax, &ebx, &ecx, &edx);
    
    printf("CPUID Leaf 2: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n",
           eax, ebx, ecx, edx);
    
    /* Query CPUID leaf 4 (Deterministic Cache Parameters) */
    for (int i = 0; i < 4; i++) {
        cpuid_query(4, &eax, &ebx, &ecx, &edx);
        printf("CPUID Leaf 4[%d]: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n",
               i, eax, ebx, ecx, edx);
    }
}

/* ============================================
   PATTERN 4: Cache-Sized Data Structures
   ============================================ */

/* Arrays sized to match specific cache sizes from the switch cases */
#define SIZE_8KB    (8192 / sizeof(int))    /* 0x0a: 8KB L1 */
#define SIZE_16KB   (16384 / sizeof(int))   /* 0x0c, 0x0d: 16KB L1 */
#define SIZE_32KB   (32768 / sizeof(int))   /* 0x2c: 32KB L1 */
#define SIZE_256KB  (262144 / sizeof(int))  /* 0x21: 256KB L2 */
#define SIZE_1MB    (1048576 / sizeof(int)) /* 0x24: 1MB L2 */

static void cache_size_optimized_loop(int* data, int size) {
    /* Tile loops for cache efficiency */
    const int tile = 64; /* Assume 64-byte cache line (16 ints) */
    for (int i = 0; i < size; i += tile) {
        int limit = (i + tile) < size ? (i + tile) : size;
        for (int j = i; j < limit; j++) {
            data[j] = data[j] * 2 + (j % 16);
        }
    }
}

#else /* Non-x86 fallback */

/* Dummy implementations for non-x86 platforms */
void cache_sensitive_computation(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] + 1;
    }
}

void compute_dynamic(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

#endif /* x86 guard */

/* ============================================
   Main Program with CPU Detection
   ============================================ */

int main(void) {
#if defined(__i386__) || defined(__x86_64__)
    printf("=== Main CPU Detection ===\n");
    
    /* Force driver to initialize CPU detection structures */
    __builtin_cpu_init();
    
    /* Extensive use of __builtin_cpu_is to trigger detection */
    const char* cpu_types[] = {
        "core2", "nehalem", "sandybridge", "ivybridge",
        "haswell", "skylake", "atom", "silvermont"
    };
    
    printf("CPU Identification:\n");
    for (size_t i = 0; i < sizeof(cpu_types)/sizeof(cpu_types[0]); i++) {
        if (__builtin_cpu_is(cpu_types[i])) {
            printf("  Detected: %s\n", cpu_types[i]);
        }
    }
    
    /* Check specific features that require cache awareness */
    printf("CPU Features:\n");
    const char* features[] = {"sse2", "sse4.2", "avx", "avx2", "fma"};
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf("  Supports: %s\n", features[i]);
        }
    }
    
    /* Get system cache information (triggers driver cache detection) */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 Cache Line: %ld bytes\n", cache_line);
    
    /* Compile-time assertion about SSE2 (common for x86) */
    _Static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, 
                   "Pointer size check");
    
    /* Create and process cache-sized arrays */
    const int test_size = SIZE_1MB; /* Use largest size */
    int* data = (int*)malloc(test_size * sizeof(int));
    if (!data) {
        perror("malloc");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < test_size; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Execute all cache-sensitive computations */
    printf("\n=== Running Cache-Sensitive Computations ===\n");
    
    /* Pattern 1: Multiversioning */
    cache_sensitive_computation(data, test_size);
    
    /* Pattern 2: ifunc dispatch */
    compute_dynamic(data, test_size);
    
    /* Pattern 4: Cache-sized loops */
    cache_size_optimized_loop(data, test_size);
    
    /* Call architecture-specific functions */
#if defined(__i386__) || defined(__x86_64__)
    core2_optimized(data, test_size);
    nehalem_optimized(data, test_size);
    sandybridge_optimized(data, test_size);
#endif
    
    /* Verify computation */
    int checksum = 0;
    for (int i = 0; i < test_size; i++) {
        checksum ^= data[i];
    }
    printf("Final checksum: 0x%08x\n", checksum);
    
    free(data);
    
#else
    printf("Non-x86 platform - running fallback code\n");
    int dummy[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    cache_sensitive_computation(dummy, 10);
    compute_dynamic(dummy, 10);
#endif
    
    return 0;
}
