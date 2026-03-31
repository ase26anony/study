/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPU cache detection logic
 * by using various CPU feature queries, function multiversioning,
 * and cache-sensitive computations.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

/* Guard for x86-specific code */
#if defined(__i386__) || defined(__x86_64__)

/* Pattern A: Function multiversioning with different target attributes */
/* Core2 architecture - may trigger cache descriptors like 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void compute_core2(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 1;
    }
}

/* Nehalem architecture - may trigger cache descriptors like 0x78, 0x79, 0x7a */
__attribute__((target("arch=nehalem")))
void compute_nehalem(int* data, int size) {
    for (int i = 0; i < size; i += 8) {
        /* Process 8 elements at a time with prefetching */
        __builtin_prefetch(&data[i + 16], 0, 3);
        data[i] = data[i] * 5 - 2;
    }
}

/* Sandy Bridge architecture - may trigger cache descriptors like 0x7b, 0x7c, 0x7d */
__attribute__((target("arch=sandybridge")))
void compute_sandybridge(int* data, int size) {
    /* Use AVX-optimized pattern if available */
    for (int i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 32], 0, 2);
        data[i] = data[i] * 7 + 3;
    }
}

/* Ivy Bridge architecture - may trigger various L2 cache descriptors */
__attribute__((target("arch=ivybridge")))
void compute_ivybridge(int* data, int size) {
    for (int i = 0; i < size; i += 32) {
        __builtin_prefetch(&data[i + 64], 0, 1);
        data[i] = data[i] * 11 - 5;
    }
}

/* Pattern B: ifunc resolver for runtime dispatch */
static void (*compute_func)(int*, int);

static void compute_default(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

static void (*resolve_compute(void))(int*, int) {
    /* Force CPU detection by checking various features */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return compute_sandybridge;
    } else if (__builtin_cpu_supports("avx")) {
        return compute_nehalem;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_ivybridge;
    } else if (__builtin_cpu_supports("sse2")) {
        return compute_core2;
    }
    
    return compute_default;
}

/* Apply ifunc attribute */
void compute_optimized(int* data, int size) 
    __attribute__((ifunc("resolve_compute")));

/* Pattern C: Direct CPUID queries */
static void cpuid_query(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Query CPUID leaf 0 to get vendor string */
    asm volatile ("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(0));
    
    /* Query CPUID leaf 1 for basic features */
    asm volatile ("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(1));
    
    /* Query CPUID leaf 2 for cache descriptors (Intel-specific) */
    asm volatile ("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(2));
    
    /* Query CPUID leaf 4 for deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile ("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(4), "c"(i));
    }
}

/* Pattern D: Cache-sensitive computations with different sizes */
/* 8KB array - matches L1 cache size for some descriptors (0x0a, 0x66) */
#define SIZE_8KB (8192 / sizeof(int))
/* 16KB array - matches L1 cache size for descriptors (0x0c, 0x0d, 0x67) */
#define SIZE_16KB (16384 / sizeof(int))
/* 32KB array - matches L1 cache size for descriptors (0x2c, 0x68) */
#define SIZE_32KB (32768 / sizeof(int))
/* 256KB array - matches L2 cache size for many descriptors */
#define SIZE_256KB (262144 / sizeof(int))

static void cache_sensitive_compute(void) {
    /* Allocate arrays of different cache-relevant sizes */
    static int data_8kb[SIZE_8KB];
    static int data_16kb[SIZE_16KB];
    static int data_32kb[SIZE_32KB];
    static int data_256kb[SIZE_256KB];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE_8KB; i++) data_8kb[i] = i & 0xFF;
    for (int i = 0; i < SIZE_16KB; i++) data_16kb[i] = i & 0xFF;
    for (int i = 0; i < SIZE_32KB; i++) data_32kb[i] = i & 0xFF;
    for (int i = 0; i < SIZE_256KB; i++) data_256kb[i] = i & 0xFF;
    
    /* Perform computations with different access patterns */
    
    /* Sequential access - good for prefetching */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < SIZE_8KB; i++) {
            data_8kb[i] = data_8kb[i] * 3 + 1;
        }
    }
    
    /* Strided access with 64-byte stride (cache line size) */
    for (int iter = 0; iter < 50; iter++) {
        for (int i = 0; i < SIZE_16KB; i += 16) { /* 16 ints = 64 bytes */
            data_16kb[i] = data_16kb[i] * 5 - 2;
        }
    }
    
    /* Blocked matrix-style access pattern */
    for (int iter = 0; iter < 20; iter++) {
        const int block = 32; /* 32*4 = 128 bytes */
        for (int i = 0; i < SIZE_32KB; i += block) {
            for (int j = 0; j < block && i + j < SIZE_32KB; j++) {
                data_32kb[i + j] = data_32kb[i + j] * 7 + 3;
            }
        }
    }
    
    /* Large array with temporal locality */
    for (int iter = 0; iter < 10; iter++) {
        for (int i = 0; i < SIZE_256KB; i++) {
            data_256kb[i] = (data_256kb[i] * 11 - 5) & 0xFFFF;
        }
    }
}

/* Constructor to run CPUID queries early */
__attribute__((constructor))
static void init_cpu_info(void) {
    printf("Initializing CPU cache detection...\n");
    cpuid_query();
    
    /* Force CPU feature detection */
    __builtin_cpu_init();
    
    /* Check various CPU features to trigger detection */
    int has_sse2 = __builtin_cpu_supports("sse2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    
    printf("CPU Features: SSE2=%d, SSE4.2=%d, AVX=%d, AVX2=%d\n",
           has_sse2, has_sse4, has_avx, has_avx2);
    
    /* Check specific Intel CPUs */
    if (__builtin_cpu_is("intel")) {
        printf("Intel CPU detected\n");
    }
    if (__builtin_cpu_is("core2")) {
        printf("Core2 microarchitecture detected\n");
    }
    if (__builtin_cpu_is("nehalem")) {
        printf("Nehalem microarchitecture detected\n");
    }
    if (__builtin_cpu_is("sandybridge")) {
        printf("Sandy Bridge microarchitecture detected\n");
    }
}

#else
/* Non-x86 fallback implementations */
void compute_optimized(int* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2;
    }
}

static void cache_sensitive_compute(void) {
    printf("Non-x86 architecture - using generic computation\n");
}

__attribute__((constructor))
static void init_cpu_info(void) {
    printf("Non-x86 architecture - skipping CPU cache detection\n");
}
#endif

/* Compile-time assertion for x86 */
#if defined(__i386__) || defined(__x86_64__)
_Static_assert(__builtin_cpu_supports("sse2"), "SSE2 required for x86");
#endif

/* Main function with cache-sensitive computation */
int main(void) {
    printf("Cache Detection Test Program\n");
    
    /* Get system cache information */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("System L1 cache line size: %ld bytes\n", cache_line);
    
    /* Perform cache-sensitive computations */
    cache_sensitive_compute();
    
    /* Test function multiversioning */
    int test_data[1024];
    for (int i = 0; i < 1024; i++) {
        test_data[i] = i;
    }
    
    compute_optimized(test_data, 1024);
    
    /* Verify computation */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += test_data[i];
    }
    
    printf("Computation checksum: %d\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
