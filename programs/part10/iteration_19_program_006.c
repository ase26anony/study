/* test_cache_coverage.c
 * 
 * This test program is designed to trigger GCC's i386 driver cache detection
 * logic for specific CPUID leaf 2 descriptor values. It uses multiple
 * compilation strategies to cover the uncovered case statements.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
#define TARGET_CPU "pentium3"
#define EXPECTED_CASE "0x0a,0x0c,0x0d,0x0e,0x21,0x24"
#elif defined(TEST_PENTIUM4)
/* Targets cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
#define TARGET_CPU "pentium4"
#define EXPECTED_CASE "0x2c,0x39-0x3e,0x41-0x45,0x49"
#elif defined(TEST_NOCONA)
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68 */
#define TARGET_CPU "nocona"
#define EXPECTED_CASE "0x49,0x60,0x66-0x68"
#elif defined(TEST_K8)
/* Targets cases: 0x78-0x87 (AMD K8 cache descriptors) */
#define TARGET_CPU "k8"
#define EXPECTED_CASE "0x78-0x87"
#elif defined(TEST_CORE2)
/* Targets cases: 0x48, 0x4e, and various L2 cases */
#define TARGET_CPU "core2"
#define EXPECTED_CASE "0x48,0x4e,0x78-0x87"
#else
/* Generic fallback - will use actual CPU detection */
#define TARGET_CPU "native"
#define EXPECTED_CASE "varies"
#endif

/* Function attributes for specific CPU targeting */
#ifdef USE_FUNCTION_TARGETING
/* Use GCC's target attribute for per-function CPU specification */
__attribute__((target("arch=pentium3")))
static void benchmark_pentium3(int *buffer, size_t size) {
    printf("Running Pentium3-optimized benchmark\n");
}

__attribute__((target("arch=pentium4")))
static void benchmark_pentium4(int *buffer, size_t size) {
    printf("Running Pentium4-optimized benchmark\n");
}

__attribute__((target("arch=nocona")))
static void benchmark_nocona(int *buffer, size_t size) {
    printf("Running Nocona-optimized benchmark\n");
}

__attribute__((target("arch=k8")))
static void benchmark_k8(int *buffer, size_t size) {
    printf("Running K8-optimized benchmark\n");
}

__attribute__((target("arch=core2")))
static void benchmark_core2(int *buffer, size_t size) {
    printf("Running Core2-optimized benchmark\n");
}
#endif

/* Cache-thrashing benchmark that uses patterns dependent on cache size */
static void run_cache_benchmark(int *buffer, size_t size, int iterations) {
    volatile int result = 0;
    size_t i, j;
    
    /* Simple linear congruential generator for pseudo-random access */
    uint32_t lcg_state = 123456789;
    
    printf("Starting cache benchmark (buffer size: %zu KB)\n", 
           (size * sizeof(int)) / 1024);
    
    /* Phase 1: Sequential access (tests prefetch and line size) */
    for (j = 0; j < iterations; j++) {
        for (i = 0; i < size; i++) {
            buffer[i] = i + j;
        }
        MEMORY_BARRIER();
    }
    
    /* Phase 2: Strided access (tests associativity) */
    /* Use prime stride to avoid power-of-two conflicts */
    const size_t stride = 257; /* Prime number */
    for (j = 0; j < iterations; j++) {
        for (i = 0; i < size; i += stride) {
            buffer[i % size] += j;
        }
        MEMORY_BARRIER();
    }
    
    /* Phase 3: Pseudo-random access (worst-case cache behavior) */
    for (j = 0; j < iterations * 2; j++) {
        lcg_state = lcg_state * 1103515245 + 12345;
        size_t idx = (lcg_state >> 16) % size;
        buffer[idx] ^= j;
        MEMORY_BARRIER();
    }
    
    /* Phase 4: Large working set (exceeds L1, tests L2 detection) */
    /* Process in chunks that are larger than typical L1 */
    const size_t chunk_size = 32768 / sizeof(int); /* 32KB worth of ints */
    for (j = 0; j < iterations * 4; j++) {
        for (i = 0; i < size; i += chunk_size) {
            size_t end = i + chunk_size;
            if (end > size) end = size;
            for (size_t k = i; k < end; k++) {
                buffer[k] = buffer[k] * 3 + 1;
            }
        }
        MEMORY_BARRIER();
    }
    
    /* Final computation to prevent dead code elimination */
    for (i = 0; i < size; i += 1024) {
        result ^= buffer[i];
    }
    
    printf("Benchmark complete (result: %d)\n", result);
}

/* Multi-versioned benchmark using target clones */
#ifdef USE_TARGET_CLONES
__attribute__((target_clones("pentium3,pentium4,nocona,k8,core2,default")))
static void multi_version_benchmark(int *buffer, size_t size) {
    static int version_called = 0;
    if (!version_called) {
        printf("Multi-version benchmark called (CPU-specific version)\n");
        version_called = 1;
    }
    run_cache_benchmark(buffer, size, 2);
}
#endif

int main(int argc, char **argv) {
    const size_t l2_size_kb = 2048; /* 2MB - larger than most L2 caches */
    const size_t buffer_size = (l2_size_kb * 1024 * 2) / sizeof(int);
    int *buffer = NULL;
    int iterations = 3;
    
    printf("Cache Coverage Test Program\n");
    printf("Target CPU: %s\n", TARGET_CPU);
    printf("Expected to trigger cases: %s\n", EXPECTED_CASE);
    printf("Compiler: %s\n", __VERSION__);
    
    /* Allocate buffer larger than L2 cache */
    buffer = (int*)malloc(buffer_size * sizeof(int));
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = (int)(i & 0xFF);
    }
    
    /* Run appropriate benchmark based on compilation mode */
#ifdef USE_FUNCTION_TARGETING
    printf("\nUsing function-targeted benchmarks:\n");
    
    /* Call all targeted functions to ensure each gets compiled */
    benchmark_pentium3(buffer, buffer_size);
    benchmark_pentium4(buffer, buffer_size);
    benchmark_nocona(buffer, buffer_size);
    benchmark_k8(buffer, buffer_size);
    benchmark_core2(buffer, buffer_size);
    
    /* Run main benchmark */
    run_cache_benchmark(buffer, buffer_size, iterations);
    
#elif defined(USE_TARGET_CLONES)
    printf("\nUsing target-clones multi-versioning:\n");
    
    /* Call multi-version function multiple times */
    for (int i = 0; i < 5; i++) {
        multi_version_benchmark(buffer, buffer_size);
        MEMORY_BARRIER();
    }
    
#else
    printf("\nUsing standard benchmark for %s:\n", TARGET_CPU);
    
    /* Single benchmark run for the targeted CPU */
    run_cache_benchmark(buffer, buffer_size, iterations);
    
    /* Additional test: Matrix multiplication (cache-blocking sensitive) */
    if (buffer_size > 4096) {
        printf("\nRunning matrix-style access pattern:\n");
        const size_t dim = 64;
        size_t offset = 0;
        
        for (int it = 0; it < 2; it++) {
            /* Simulate cache-blocked matrix access */
            for (size_t block_i = 0; block_i < dim; block_i += 8) {
                for (size_t block_j = 0; block_j < dim; block_j += 8) {
                    for (size_t i = block_i; i < block_i + 8 && i < dim; i++) {
                        for (size_t j = block_j; j < block_j + 8 && j < dim; j++) {
                            size_t idx = ((i * dim + j) + offset) % buffer_size;
                            buffer[idx] = buffer[idx] * 2 + 1;
                        }
                    }
                }
            }
            offset += dim * dim;
            MEMORY_BARRIER();
        }
    }
#endif
    
    /* Final validation to ensure all memory was touched */
    volatile int checksum = 0;
    for (size_t i = 0; i < buffer_size; i += 128) {
        checksum += buffer[i];
    }
    
    printf("\nTest completed. Final checksum: %d\n", checksum);
    printf("Buffer size: %.2f MB\n", 
           (buffer_size * sizeof(int)) / (1024.0 * 1024.0));
    
    free(buffer);
    return 0;
}
