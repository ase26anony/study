/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPUID leaf 2 cache detection
 * logic by compiling for specific x86 microarchitectures. The program
 * performs cache-intensive operations that should cause GCC to examine
 * and use the cache parameters for optimization decisions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
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
static void cache_intensive_benchmark(int iterations) {
    /* Allocate buffers larger than typical L2 cache */
    const size_t buffer_size = 4 * 1024 * 1024; /* 4MB */
    volatile int* buffer1 = (volatile int*)malloc(buffer_size * sizeof(int));
    volatile int* buffer2 = (volatile int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize buffers with pseudo-random data */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        /* Simple LCG for pseudo-random values */
        seed = seed * 1103515245 + 12345;
        buffer1[i] = (int)(seed & 0x7FFFFFFF);
        buffer2[i] = (int)((seed >> 16) & 0x7FFFFFFF);
    }
    
    MEMORY_BARRIER();
    
    volatile int result = 0;
    
    /* Cache-thrashing benchmark with different access patterns */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pattern 1: Sequential access (good for prefetch) */
        for (size_t i = 0; i < buffer_size; i += 64) {
            result += buffer1[i];
            buffer2[i] = result;
        }
        
        MEMORY_BARRIER();
        
        /* Pattern 2: Strided access (tests associativity) */
        const size_t stride = 1024; /* Large stride to cause conflicts */
        for (size_t i = 0; i < buffer_size; i += stride) {
            result ^= buffer1[i];
            buffer2[(i + 512) % buffer_size] = result;
        }
        
        MEMORY_BARRIER();
        
        /* Pattern 3: Pseudo-random access (worst-case for cache) */
        seed = iter;
        for (size_t j = 0; j < buffer_size / 4; j++) {
            seed = seed * 1103515245 + 12345;
            size_t idx = seed % buffer_size;
            result += buffer1[idx];
            buffer2[idx] = result;
        }
        
        MEMORY_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    printf("Benchmark result: %d\n", result);
    
    free((void*)buffer1);
    free((void*)buffer2);
}

/* Multi-versioned function using target clones */
#if defined(USE_MULTIVERSIONING) && __GNUC__ >= 6
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void multiarch_cache_test(int iterations) {
    cache_intensive_benchmark(iterations);
}

/* Specialized functions for specific cache descriptor cases */
#ifdef TEST_CASE_0x0A
/* 8KB L1, 2-way, 32B line - Pentium III */
__attribute__((target("arch=pentium3")))
void test_case_0x0a(void) {
    printf("Testing for descriptor 0x0a (Pentium III style)\n");
    cache_intensive_benchmark(10);
}
#endif

#ifdef TEST_CASE_0x49
/* 4MB L2, 16-way, 64B line - Pentium 4/Xeon (non-MP) */
__attribute__((target("arch=nocona")))
void test_case_0x49(void) {
    printf("Testing for descriptor 0x49 (Xeon DP/Nocona)\n");
    /* Use larger working set to exercise L2 cache */
    const size_t huge_size = 8 * 1024 * 1024;
    volatile int* huge_buffer = (volatile int*)malloc(huge_size * sizeof(int));
    
    if (huge_buffer) {
        for (size_t i = 0; i < huge_size; i++) {
            huge_buffer[i] = i;
        }
        
        volatile int sum = 0;
        for (int iter = 0; iter < 5; iter++) {
            for (size_t i = 0; i < huge_size; i += 128) {
                sum += huge_buffer[i];
            }
        }
        printf("Large buffer sum: %d\n", sum);
        free((void*)huge_buffer);
    }
}
#endif

#ifdef TEST_CASE_0x78_0x87
/* Various L2 configurations - AMD K8 family */
__attribute__((target("arch=k8")))
void test_case_k8_family(void) {
    printf("Testing for descriptors 0x78-0x87 (AMD K8 family)\n");
    
    /* Test different working set sizes to trigger different optimizations */
    for (int ws = 1; ws <= 4; ws++) {
        size_t size = ws * 1024 * 1024;
        volatile int* buf = (volatile int*)malloc(size * sizeof(int));
        
        if (buf) {
            /* Initialize */
            for (size_t i = 0; i < size; i++) {
                buf[i] = (i * 3) & 0xFF;
            }
            
            /* Matrix-style access pattern */
            volatile int total = 0;
            const size_t cols = 1024;
            const size_t rows = size / cols;
            
            for (size_t r = 0; r < rows; r++) {
                for (size_t c = 0; c < cols; c++) {
                    total += buf[r * cols + c];
                }
            }
            
            printf("WS=%dMB total=%d\n", ws, total);
            free((void*)buf);
        }
    }
}
#endif

int main(int argc, char** argv) {
    int iterations = 5;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 5;
    }
    
    printf("Cache detection test program\n");
    printf("Compiled for architecture targeting specific CPUID leaf 2 cases\n\n");
    
    /* Execute architecture-specific tests based on compilation flags */
    
#ifdef TEST_CASE_0x0A
    test_case_0x0a();
#endif

#ifdef TEST_CASE_0x49
    test_case_0x49();
#endif

#ifdef TEST_CASE_0x78_0x87
    test_case_k8_family();
#endif

#if defined(USE_MULTIVERSIONING) && __GNUC__ >= 6
    printf("Running multi-versioned cache test:\n");
    multiarch_cache_test(iterations);
#else
    printf("Running cache-intensive benchmark:\n");
    cache_intensive_benchmark(iterations);
#endif
    
    /* Additional test: matrix multiplication to exercise cache hierarchy */
    printf("\nRunning matrix multiplication test:\n");
    const int N = 512;
    volatile int* A = (volatile int*)malloc(N * N * sizeof(int));
    volatile int* B = (volatile int*)malloc(N * N * sizeof(int));
    volatile int* C = (volatile int*)malloc(N * N * sizeof(int));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < N * N; i++) {
            A[i] = i % 100;
            B[i] = (i * 7) % 100;
            C[i] = 0;
        }
        
        MEMORY_BARRIER();
        
        /* Blocked matrix multiplication (cache-aware) */
        const int BLOCK = 32;
        volatile int checksum = 0;
        
        for (int i0 = 0; i0 < N; i0 += BLOCK) {
            for (int j0 = 0; j0 < N; j0 += BLOCK) {
                for (int k0 = 0; k0 < N; k0 += BLOCK) {
                    for (int i = i0; i < i0 + BLOCK && i < N; i++) {
                        for (int j = j0; j < j0 + BLOCK && j < N; j++) {
                            int sum = C[i * N + j];
                            for (int k = k0; k < k0 + BLOCK && k < N; k++) {
                                sum += A[i * N + k] * B[k * N + j];
                            }
                            C[i * N + j] = sum;
                        }
                    }
                }
            }
        }
        
        /* Compute checksum */
        for (int i = 0; i < N * N; i += 97) {
            checksum ^= C[i];
        }
        
        printf("Matrix checksum: %d\n", checksum);
        
        free((void*)A);
        free((void*)B);
        free((void*)C);
    }
    
    return 0;
}
