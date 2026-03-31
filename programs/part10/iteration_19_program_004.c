/* test_cache_detection.c - Comprehensive test for GCC i386 driver cache detection */
/* Compile with different -march flags to trigger specific cache descriptor cases */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different architecture targets to trigger specific cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
/* Targets cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
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
static void cache_thrashing_benchmark(int iterations) {
    /* Allocate buffers larger than typical L2 cache */
    const size_t buffer_size = 4 * 1024 * 1024; /* 4MB */
    volatile int* buffer1 = (volatile int*)malloc(buffer_size * sizeof(int));
    volatile int* buffer2 = (volatile int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        seed = seed * 1103515245 + 12345;
        buffer1[i] = (int)(seed & 0x7FFFFFFF);
        buffer2[i] = (int)(seed >> 16);
    }
    
    MEMORY_BARRIER();
    
    volatile int result = 0;
    
    /* Cache-thrashing access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        /* Strided access to defeat prefetching */
        for (size_t i = 0; i < buffer_size; i += 64) {
            result ^= buffer1[i];
            buffer2[i] = result;
        }
        
        /* Reverse strided access */
        for (size_t i = buffer_size - 1; i > 0; i -= 128) {
            result ^= buffer2[i];
            buffer1[i] = result;
        }
        
        MEMORY_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    printf("Benchmark result: %d\n", result);
    
    free((void*)buffer1);
    free((void*)buffer2);
}

/* Multi-versioned function using target_clones */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void multiarch_cache_test(int scale) {
    /* Different buffer sizes to trigger different cache behaviors */
    size_t sizes[] = {8 * 1024, 16 * 1024, 32 * 1024, 64 * 1024, 
                      128 * 1024, 256 * 1024, 512 * 1024, 1024 * 1024};
    
    volatile int total = 0;
    
    for (size_t s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        size_t size = sizes[s] * scale;
        volatile int* buf = (volatile int*)malloc(size * sizeof(int));
        
        if (!buf) continue;
        
        /* Initialize */
        for (size_t i = 0; i < size; i++) {
            buf[i] = (int)(i ^ 0x55555555);
        }
        
        MEMORY_BARRIER();
        
        /* Access with prime number stride to avoid power-of-two issues */
        for (size_t i = 0; i < size; i = (i + 997) % size) {
            total += buf[i];
            buf[i] = total;
        }
        
        MEMORY_BARRIER();
        
        free((void*)buf);
    }
    
    printf("Multi-arch total: %d\n", total);
}

/* Special test for case 0x49 (Xeon MP vs non-Xeon MP) */
#ifdef TEST_XEON_MP
__attribute__((target("arch=pentium4")))
static void xeon_mp_test(void) {
    /* This should trigger case 0x49 with xeon_mp flag set */
    volatile int* large_buf = (volatile int*)malloc(8 * 1024 * 1024 * sizeof(int));
    
    if (large_buf) {
        /* Access pattern that benefits from large cache */
        for (int i = 0; i < 1000; i++) {
            for (size_t j = 0; j < 1024 * 1024; j += 256) {
                large_buf[j] = i;
            }
            MEMORY_BARRIER();
        }
        free((void*)large_buf);
    }
}
#endif

int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Starting cache detection test...\n");
    
    /* Call architecture-specific tests based on compilation flags */
    
#if defined(TEST_PENTIUM3)
    printf("Testing Pentium III cache configuration...\n");
    cache_thrashing_benchmark(iterations);
    
#elif defined(TEST_PENTIUM4)
    printf("Testing Pentium 4 cache configuration...\n");
    cache_thrashing_benchmark(iterations);
    
#elif defined(TEST_NOCONA)
    printf("Testing Nocona (Xeon DP) cache configuration...\n");
    cache_thrashing_benchmark(iterations);
    
#elif defined(TEST_K8)
    printf("Testing AMD K8 cache configuration...\n");
    cache_thrashing_benchmark(iterations);
    
#elif defined(TEST_CORE2)
    printf("Testing Core 2 cache configuration...\n");
    cache_thrashing_benchmark(iterations);
    
#elif defined(TEST_XEON_MP)
    printf("Testing Xeon MP cache configuration...\n");
    xeon_mp_test();
    
#else
    /* Generic test - will use native CPU detection */
    printf("Testing with native CPU detection...\n");
    cache_thrashing_benchmark(iterations);
    
    /* Also test multi-versioning if supported */
    #ifdef USE_MULTIVERSIONING
    printf("Testing multi-versioned function...\n");
    multiarch_cache_test(2);
    #endif
#endif
    
    /* Additional test with matrix multiplication to stress cache hierarchy */
    printf("Running matrix multiplication stress test...\n");
    const int N = 512;
    volatile double* A = (volatile double*)malloc(N * N * sizeof(double));
    volatile double* B = (volatile double*)malloc(N * N * sizeof(double));
    volatile double* C = (volatile double*)malloc(N * N * sizeof(double));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < N * N; i++) {
            A[i] = (double)(i % 100);
            B[i] = (double)((i + 1) % 100);
            C[i] = 0.0;
        }
        
        MEMORY_BARRIER();
        
        /* Blocked matrix multiplication to trigger cache-aware optimizations */
        const int BLOCK = 32;
        for (int i = 0; i < N; i += BLOCK) {
            for (int j = 0; j < N; j += BLOCK) {
                for (int k = 0; k < N; k += BLOCK) {
                    for (int ii = i; ii < i + BLOCK && ii < N; ii++) {
                        for (int jj = j; jj < j + BLOCK && jj < N; jj++) {
                            double sum = C[ii * N + jj];
                            for (int kk = k; kk < k + BLOCK && kk < N; kk++) {
                                sum += A[ii * N + kk] * B[kk * N + jj];
                            }
                            C[ii * N + jj] = sum;
                        }
                    }
                }
            }
        }
        
        MEMORY_BARRIER();
        
        /* Compute checksum */
        double checksum = 0.0;
        for (int i = 0; i < N * N; i += 97) {
            checksum += C[i];
        }
        printf("Matrix checksum: %f\n", checksum);
        
        free((void*)A);
        free((void*)B);
        free((void*)C);
    }
    
    return 0;
}
