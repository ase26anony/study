/* test_cache_descriptors.c - Cover GCC i386 driver cache descriptor cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our memory accesses */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
/* Targets cases: 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x41-0x45 */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68 */
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
/* Targets cases: 0x78-0x87 */
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
/* Targets cases: 0x48, 0x4e */
__attribute__((target("arch=core2")))
#endif
static void benchmark_cache(int* buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Simple LCG for pseudo-random access pattern */
    uint32_t rng_state = 0xDEADBEEF;
    
    for (j = 0; j < iterations; j++) {
        /* Sequential access (good for prefetch) */
        for (i = 0; i < size; i++) {
            buffer[i] = i + j;
        }
        COMPILER_BARRIER();
        
        /* Strided access (tests associativity) */
        for (i = 0; i < size; i += 17) {
            sink += buffer[i];
        }
        COMPILER_BARRIER();
        
        /* Pseudo-random access (thrashes cache) */
        for (i = 0; i < size * 4; i++) {
            rng_state = rng_state * 1103515245 + 12345;
            size_t idx = (rng_state >> 16) % size;
            buffer[idx] = sink;
            sink += buffer[idx];
        }
        COMPILER_BARRIER();
    }
    
    /* Ensure sink is used */
    if (sink == 0x12345678) {
        printf("Impossible\n");
    }
}

/* Multi-versioned function using target_clones */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void multiarch_benchmark(int* buffer, size_t size) {
    benchmark_cache(buffer, size, 3);
}

int main(int argc, char** argv) {
    const size_t l1_size = 32 * 1024 / sizeof(int);      /* ~32KB */
    const size_t l2_size = 512 * 1024 / sizeof(int);     /* ~512KB */
    const size_t huge_size = 4 * 1024 * 1024 / sizeof(int); /* 4MB */
    
    int *buffer_small, *buffer_medium, *buffer_large;
    volatile int result = 0;
    clock_t start, end;
    
    /* Allocate buffers of different sizes to test different cache levels */
    buffer_small = (int*)malloc(l1_size * sizeof(int));
    buffer_medium = (int*)malloc(l2_size * sizeof(int));
    buffer_large = (int*)malloc(huge_size * sizeof(int));
    
    if (!buffer_small || !buffer_medium || !buffer_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (size_t i = 0; i < l1_size; i++) buffer_small[i] = i;
    for (size_t i = 0; i < l2_size; i++) buffer_medium[i] = i;
    for (size_t i = 0; i < huge_size; i++) buffer_large[i] = i;
    
    printf("Starting cache benchmark...\n");
    
    /* Test different access patterns to engage cache optimization logic */
    
    /* Pattern 1: Sequential access - good spatial locality */
    start = clock();
    for (int iter = 0; iter < 10; iter++) {
        for (size_t i = 0; i < l2_size; i++) {
            buffer_medium[i] = buffer_medium[i] * 3 + 7;
        }
        COMPILER_BARRIER();
    }
    end = clock();
    printf("Sequential: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Pattern 2: Blocked matrix-style access - tests cache blocking optimizations */
    start = clock();
    const size_t block = 64; /* 64*4=256 bytes - typical cache line */
    for (int iter = 0; iter < 5; iter++) {
        for (size_t i = 0; i < huge_size; i += block) {
            for (size_t j = 0; j < block && (i + j) < huge_size; j++) {
                buffer_large[i + j] = buffer_large[i + j] * 2 - 1;
            }
        }
        COMPILER_BARRIER();
    }
    end = clock();
    printf("Blocked: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Pattern 3: Random access - worst case for cache */
    start = clock();
    uint32_t seed = 0x12345678;
    for (int iter = 0; iter < 3; iter++) {
        for (size_t i = 0; i < l2_size * 2; i++) {
            seed = seed * 1664525 + 1013904223;
            size_t idx = seed % l2_size;
            result += buffer_medium[idx];
        }
        COMPILER_BARRIER();
    }
    end = clock();
    printf("Random: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Call architecture-specific benchmarks if compiled with multi-versioning */
#ifdef USE_MULTIVERSIONING
    printf("Running multi-arch benchmarks...\n");
    multiarch_benchmark(buffer_small, l1_size);
    multiarch_benchmark(buffer_medium, l2_size);
    multiarch_benchmark(buffer_large, huge_size / 4);
#endif
    
    /* Force use of all buffers to prevent optimization */
    for (size_t i = 0; i < l1_size; i += 128) result ^= buffer_small[i];
    for (size_t i = 0; i < l2_size; i += 256) result ^= buffer_medium[i];
    for (size_t i = 0; i < huge_size; i += 512) result ^= buffer_large[i];
    
    printf("Result checksum: %d\n", result);
    
    free(buffer_small);
    free(buffer_medium);
    free(buffer_large);
    
    return 0;
}
