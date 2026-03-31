/* test_cache_descriptors.c
 * 
 * This test program is designed to trigger GCC's i386 driver cache detection
 * logic for specific CPUID leaf 2 descriptor values. It uses conditional
 * compilation and function multi-versioning to target different x86
 * microarchitectures, each with specific cache configurations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different test configurations targeting specific cache descriptors */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
#define TARGET_ARCH "pentium3"
#define TARGET_TUNE "pentium3"
#elif defined(TEST_PENTIUM4)
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x39-0x3e, 0x41-0x45 */
#define TARGET_ARCH "pentium4"
#define TARGET_TUNE "pentium4"
#elif defined(TEST_NOCONA)
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68, 0x78-0x87 */
#define TARGET_ARCH "nocona"
#define TARGET_TUNE "nocona"
#elif defined(TEST_K8)
/* Targets cases: 0x40 series, 0x78-0x87 (AMD K8) */
#define TARGET_ARCH "k8"
#define TARGET_TUNE "k8"
#elif defined(TEST_CORE2)
/* Targets cases: 0x66, 0x67, 0x68, 0x78-0x87 */
#define TARGET_ARCH "core2"
#define TARGET_TUNE "core2"
#elif defined(TEST_NEHALEM)
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x2c, 0x78-0x87 */
#define TARGET_ARCH "nehalem"
#define TARGET_TUNE "nehalem"
#else
/* Generic x86-64 - will use actual CPU detection */
#define TARGET_ARCH "x86-64"
#define TARGET_TUNE "generic"
#endif

/* Function attributes for specific CPU targeting */
#if defined(__GNUC__) && (__GNUC__ >= 4)
/* Use target attribute to force specific CPU feature detection */
__attribute__((target("arch=" TARGET_ARCH ",tune=" TARGET_TUNE)))
#endif
static void cache_thrash_benchmark(int iterations) {
    /* Allocate buffers larger than typical L2 cache */
    const size_t buffer_size = 4 * 1024 * 1024; /* 4 MB */
    volatile int* buffer1 = (volatile int*)malloc(buffer_size * sizeof(int));
    volatile int* buffer2 = (volatile int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        /* Simple LCG for pseudo-random values */
        seed = seed * 1103515245 + 12345;
        buffer1[i] = (int)(seed & 0x7FFFFFFF);
        buffer2[i] = (int)(seed >> 16);
    }
    
    MEMORY_BARRIER();
    
    volatile int result = 0;
    
    /* Cache-thrashing memory access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        /* Strided access pattern to test different cache associativities */
        for (size_t i = 0; i < buffer_size; i += 64) {
            result ^= buffer1[i];
            result += buffer2[i];
        }
        
        /* Reverse direction to hit different cache lines */
        for (size_t i = buffer_size - 1; i > 0; i -= 128) {
            result ^= buffer2[i];
            result += buffer1[i];
        }
        
        /* Random-like access using simple hash */
        for (size_t i = 0; i < buffer_size; i++) {
            size_t idx = (i * 97) % buffer_size; /* Prime multiplier */
            result ^= buffer1[idx];
            idx = (idx * 113) % buffer_size; /* Another prime */
            result += buffer2[idx];
        }
        
        MEMORY_BARRIER();
    }
    
    /* Use result to prevent optimization */
    printf("Benchmark result: %d\n", result);
    
    free((void*)buffer1);
    free((void*)buffer2);
}

/* Additional test functions for specific cache descriptor cases */
#if defined(__GNUC__) && (__GNUC__ >= 6)
/* Multi-versioned function for different CPU targets */
__attribute__((target_clones("arch=pentium3,tune=pentium3",
                             "arch=pentium4,tune=pentium4",
                             "arch=nocona,tune=nocona",
                             "arch=k8,tune=k8",
                             "arch=core2,tune=core2",
                             "arch=nehalem,tune=nehalem",
                             "default")))
static void multi_version_cache_test(int iterations) {
    cache_thrash_benchmark(iterations);
}
#endif

/* Matrix multiplication to stress cache hierarchy */
#if defined(__GNUC__) && (__GNUC__ >= 4)
__attribute__((target("arch=" TARGET_ARCH ",tune=" TARGET_TUNE)))
#endif
static void matrix_cache_test(void) {
    const int N = 512; /* Size that exceeds L1 but fits in L2 */
    volatile double* A = (volatile double*)malloc(N * N * sizeof(double));
    volatile double* B = (volatile double*)malloc(N * N * sizeof(double));
    volatile double* C = (volatile double*)malloc(N * N * sizeof(double));
    
    if (!A || !B || !C) {
        fprintf(stderr, "Matrix allocation failed\n");
        exit(1);
    }
    
    /* Initialize matrices */
    for (int i = 0; i < N * N; i++) {
        A[i] = (double)(i % 100);
        B[i] = (double)((i + 1) % 100);
        C[i] = 0.0;
    }
    
    MEMORY_BARRIER();
    
    /* Blocked matrix multiplication to test cache behavior */
    const int BLOCK_SIZE = 32; /* Typical cache line aware block size */
    volatile double sum = 0.0;
    
    for (int i = 0; i < N; i += BLOCK_SIZE) {
        for (int j = 0; j < N; j += BLOCK_SIZE) {
            for (int k = 0; k < N; k += BLOCK_SIZE) {
                /* Mini-block multiplication */
                for (int ii = i; ii < i + BLOCK_SIZE && ii < N; ii++) {
                    for (int jj = j; jj < j + BLOCK_SIZE && jj < N; jj++) {
                        double temp = 0.0;
                        for (int kk = k; kk < k + BLOCK_SIZE && kk < N; kk++) {
                            temp += A[ii * N + kk] * B[kk * N + jj];
                        }
                        C[ii * N + jj] += temp;
                    }
                }
            }
        }
    }
    
    /* Sum elements to prevent optimization */
    for (int i = 0; i < N * N; i++) {
        sum += C[i];
    }
    
    printf("Matrix test sum: %f\n", (double)sum);
    
    free((void*)A);
    free((void*)B);
    free((void*)C);
}

int main(int argc, char** argv) {
    int iterations = 10;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    
    printf("Testing cache detection for target: %s (tune: %s)\n", 
           TARGET_ARCH, TARGET_TUNE);
    printf("Running %d iterations...\n", iterations);
    
    /* Run cache thrashing benchmark */
    cache_thrash_benchmark(iterations);
    
    /* Run matrix multiplication test */
    matrix_cache_test();
    
#if defined(__GNUC__) && (__GNUC__ >= 6)
    /* Run multi-version test if supported */
    printf("Running multi-version test...\n");
    multi_version_cache_test(iterations / 2);
#endif
    
    /* Additional test with different access patterns */
    printf("Running varied stride test...\n");
    
    const size_t test_size = 2 * 1024 * 1024; /* 2MB */
    volatile int* test_array = (volatile int*)malloc(test_size * sizeof(int));
    
    if (!test_array) {
        fprintf(stderr, "Test array allocation failed\n");
        return 1;
    }
    
    /* Initialize */
    for (size_t i = 0; i < test_size; i++) {
        test_array[i] = (int)(i % 256);
    }
    
    MEMORY_BARRIER();
    
    /* Test different strides to hit various cache configurations */
    volatile int total = 0;
    for (int stride = 1; stride <= 64; stride *= 2) {
        for (size_t i = 0; i < test_size; i += stride) {
            total ^= test_array[i];
        }
        MEMORY_BARRIER();
    }
    
    printf("Stride test total: %d\n", total);
    
    free((void*)test_array);
    
    return 0;
}
