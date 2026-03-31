/* test_cache_coverage.c
 * 
 * This program is designed to exercise GCC's CPU cache detection logic
 * by targeting specific x86 microarchitectures that trigger various
 * CPUID leaf 2 cache descriptor values.
 * 
 * Compile with different -march flags to test different cache configurations:
 *   gcc -O2 -march=pentium3 -DARCH_PENTIUM3 test_cache_coverage.c -o test_p3
 *   gcc -O2 -march=pentium4 -DARCH_PENTIUM4 test_cache_coverage.c -o test_p4
 *   gcc -O2 -march=nocona -DARCH_NOCONA test_cache_coverage.c -o test_nocona
 *   gcc -O2 -march=k8 -DARCH_K8 test_cache_coverage.c -o test_k8
 *   gcc -O2 -march=core2 -DARCH_CORE2 test_cache_coverage.c -o test_core2
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Function attributes for targeting specific architectures */
#ifdef ARCH_PENTIUM3
#define TARGET_ATTR __attribute__((target("arch=pentium3")))
#elif defined(ARCH_PENTIUM4)
#define TARGET_ATTR __attribute__((target("arch=pentium4")))
#elif defined(ARCH_NOCONA)
#define TARGET_ATTR __attribute__((target("arch=nocona")))
#elif defined(ARCH_K8)
#define TARGET_ATTR __attribute__((target("arch=k8")))
#elif defined(ARCH_CORE2)
#define TARGET_ATTR __attribute__((target("arch=core2")))
#else
#define TARGET_ATTR
#endif

/* Cache thrashing benchmark function */
TARGET_ATTR
static void cache_thrash_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Use a pseudo-random access pattern to avoid prefetching */
    for (j = 0; j < iterations; j++) {
        /* Linear access with stride to exercise different cache lines */
        for (i = 0; i < size; i += 64) {  /* 64-byte stride for cache lines */
            buffer[i] = buffer[i] + 1;
        }
        
        /* Reverse access pattern */
        for (i = size - 1; i > 0; i -= 128) {  /* 128-byte stride */
            buffer[i] = buffer[i] - 1;
        }
        
        /* Random-ish access using simple LCG */
        uint32_t seed = j * 1103515245 + 12345;
        for (i = 0; i < 1000; i++) {
            seed = seed * 1103515245 + 12345;
            size_t idx = (seed >> 16) % size;
            buffer[idx] = buffer[idx] ^ seed;
        }
        
        MB();  /* Memory barrier */
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < size; i++) {
        sink ^= buffer[i];
    }
}

/* Specialized benchmark functions for different cache descriptor cases */
#ifdef ARCH_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
static void benchmark_pentium3(void) {
    printf("Testing Pentium III cache configuration...\n");
    
    /* Allocate buffers sized to exercise L1 and L2 caches */
    size_t l1_size = 16 * 1024 / sizeof(int);  /* ~16KB */
    size_t l2_size = 256 * 1024 / sizeof(int); /* ~256KB */
    
    int *buffer1 = malloc(l1_size * sizeof(int));
    int *buffer2 = malloc(l2_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Initialize buffers */
    memset(buffer1, 0xAA, l1_size * sizeof(int));
    memset(buffer2, 0x55, l2_size * sizeof(int));
    
    /* Run benchmarks */
    cache_thrash_benchmark(buffer1, l1_size, 1000);
    cache_thrash_benchmark(buffer2, l2_size, 100);
    
    free(buffer1);
    free(buffer2);
}
#endif

#ifdef ARCH_PENTIUM4
/* Targets cases: 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x41-0x45 */
__attribute__((target("arch=pentium4")))
static void benchmark_pentium4(void) {
    printf("Testing Pentium 4 cache configuration...\n");
    
    /* Pentium 4 typically had 8KB L1, 256-512KB L2 */
    size_t l1_size = 8 * 1024 / sizeof(int);
    size_t l2_size = 512 * 1024 / sizeof(int);
    
    int *buffer1 = malloc(l1_size * sizeof(int));
    int *buffer2 = malloc(l2_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    memset(buffer1, 0xCC, l1_size * sizeof(int));
    memset(buffer2, 0xDD, l2_size * sizeof(int));
    
    /* Use SSE instructions to engage Pentium 4 optimizations */
    #ifdef __SSE__
    __m128i *vec_buffer = (__m128i*)buffer2;
    for (size_t i = 0; i < l2_size / 4; i++) {
        vec_buffer[i] = _mm_set1_epi32(i);
    }
    #endif
    
    cache_thrash_benchmark(buffer1, l1_size, 2000);
    cache_thrash_benchmark(buffer2, l2_size, 200);
    
    free(buffer1);
    free(buffer2);
}
#endif

#ifdef ARCH_NOCONA
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66, 0x67, 0x68 */
__attribute__((target("arch=nocona")))
static void benchmark_nocona(void) {
    printf("Testing Nocona (Xeon DP) cache configuration...\n");
    
    /* Nocona: 16KB L1, 1MB-2MB L2 */
    size_t l1_size = 16 * 1024 / sizeof(int);
    size_t l2_size = 1024 * 1024 / sizeof(int);  /* 1MB */
    
    int *buffer1 = malloc(l1_size * sizeof(int));
    int *buffer2 = malloc(l2_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    memset(buffer1, 0x11, l1_size * sizeof(int));
    memset(buffer2, 0x22, l2_size * sizeof(int));
    
    /* Use 64-bit operations for Nocona */
    uint64_t *buffer64 = (uint64_t*)buffer2;
    for (size_t i = 0; i < l2_size / 2; i++) {
        buffer64[i] = (uint64_t)i * 0x123456789ABCDEFULL;
    }
    
    cache_thrash_benchmark(buffer1, l1_size, 1500);
    cache_thrash_benchmark(buffer2, l2_size, 150);
    
    free(buffer1);
    free(buffer2);
}
#endif

#ifdef ARCH_K8
/* Targets cases: 0x78-0x87 (AMD K8 cache descriptors) */
__attribute__((target("arch=k8")))
static void benchmark_k8(void) {
    printf("Testing AMD K8 (Athlon64) cache configuration...\n");
    
    /* K8: 64KB L1, 512KB-1MB L2 */
    size_t l1_size = 64 * 1024 / sizeof(int);
    size_t l2_size = 512 * 1024 / sizeof(int);
    
    int *buffer1 = malloc(l1_size * sizeof(int));
    int *buffer2 = malloc(l2_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    memset(buffer1, 0x33, l1_size * sizeof(int));
    memset(buffer2, 0x44, l2_size * sizeof(int));
    
    /* Use prefetch hints for AMD optimization */
    for (size_t i = 0; i < l2_size; i += 16) {
        __builtin_prefetch(&buffer2[i + 64], 0, 3);
    }
    
    cache_thrash_benchmark(buffer1, l1_size, 1200);
    cache_thrash_benchmark(buffer2, l2_size, 120);
    
    free(buffer1);
    free(buffer2);
}
#endif

#ifdef ARCH_CORE2
/* Targets cases: 0x2c, 0x48, 0x4e, 0x7a-0x7f, 0x80-0x87 */
__attribute__((target("arch=core2")))
static void benchmark_core2(void) {
    printf("Testing Core 2 cache configuration...\n");
    
    /* Core 2: 32KB L1, 2MB-4MB L2 */
    size_t l1_size = 32 * 1024 / sizeof(int);
    size_t l2_size = 2048 * 1024 / sizeof(int);  /* 2MB */
    
    int *buffer1 = malloc(l1_size * sizeof(int));
    int *buffer2 = malloc(l2_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    memset(buffer1, 0x55, l1_size * sizeof(int));
    memset(buffer2, 0x66, l2_size * sizeof(int));
    
    /* Use SSE2/SSE3 for Core 2 optimizations */
    #ifdef __SSE2__
    for (size_t i = 0; i < l1_size; i += 4) {
        __m128i v = _mm_set_epi32(i+3, i+2, i+1, i);
        _mm_store_si128((__m128i*)&buffer1[i], v);
    }
    #endif
    
    cache_thrash_benchmark(buffer1, l1_size, 1000);
    cache_thrash_benchmark(buffer2, l2_size, 100);
    
    free(buffer1);
    free(buffer2);
}
#endif

/* Generic benchmark that should work on any architecture */
static void benchmark_generic(void) {
    printf("Testing generic cache configuration...\n");
    
    /* Allocate various sized buffers to exercise different cache levels */
    size_t sizes[] = {
        8 * 1024 / sizeof(int),    /* 8KB - small L1 */
        16 * 1024 / sizeof(int),   /* 16KB - typical L1 */
        64 * 1024 / sizeof(int),   /* 64KB - large L1 */
        256 * 1024 / sizeof(int),  /* 256KB - small L2 */
        1024 * 1024 / sizeof(int), /* 1MB - typical L2 */
        2048 * 1024 / sizeof(int), /* 2MB - large L2 */
        4096 * 1024 / sizeof(int)  /* 4MB - very large L2/3 */
    };
    
    int num_buffers = sizeof(sizes) / sizeof(sizes[0]);
    int **buffers = malloc(num_buffers * sizeof(int*));
    
    if (!buffers) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Allocate and initialize buffers */
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = malloc(sizes[i] * sizeof(int));
        if (buffers[i]) {
            memset(buffers[i], i + 1, sizes[i] * sizeof(int));
        }
    }
    
    /* Access buffers in interleaved pattern to confuse prefetchers */
    volatile int result = 0;
    for (int iter = 0; iter < 50; iter++) {
        for (int buf_idx = 0; buf_idx < num_buffers; buf_idx++) {
            if (!buffers[buf_idx]) continue;
            
            size_t stride = 1 << (buf_idx % 4);  /* Varying strides */
            for (size_t i = 0; i < sizes[buf_idx]; i += stride) {
                buffers[buf_idx][i] = buffers[buf_idx][i] * 3 + 1;
            }
        }
        MB();  /* Memory barrier */
    }
    
    /* Compute final result */
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) {
            for (size_t j = 0; j < sizes[i]; j += 128) {
                result ^= buffers[i][j];
            }
            free(buffers[i]);
        }
    }
    
    free(buffers);
    printf("Generic benchmark result: %d\n", result);
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    
    /* Run architecture-specific benchmarks */
    #ifdef ARCH_PENTIUM3
    benchmark_pentium3();
    #elif defined(ARCH_PENTIUM4)
    benchmark_pentium4();
    #elif defined(ARCH_NOCONA)
    benchmark_nocona();
    #elif defined(ARCH_K8)
    benchmark_k8();
    #elif defined(ARCH_CORE2)
    benchmark_core2();
    #else
    benchmark_generic();
    #endif
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Benchmark completed in %.3f seconds\n", cpu_time_used);
    
    /* Additional test: Matrix multiplication to exercise cache blocking */
    printf("\nRunning matrix multiplication test...\n");
    const int N = 256;  /* Size that doesn't fit in L1 */
    int *A = malloc(N * N * sizeof(int));
    int *B = malloc(N * N * sizeof(int));
    int *C = malloc(N * N * sizeof(int));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < N * N; i++) {
            A[i] = i % 100;
            B[i] = (i + 1) % 100;
            C[i] = 0;
        }
        
        /* Cache-aware matrix multiplication */
        const int BLOCK_SIZE = 32;  /* Try to fit in L1 */
        for (int i0 = 0; i0 < N; i0 += BLOCK_SIZE) {
            for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                for (int k0 = 0; k0 < N; k0 += BLOCK_SIZE) {
                    for (int i = i0; i < i0 + BLOCK_SIZE && i < N; i++) {
                        for (int j = j0; j < j0 + BLOCK_SIZE && j < N; j++) {
                            int sum = C[i * N + j];
                            for (int k = k0; k < k0 + BLOCK_SIZE && k < N; k++) {
                                sum += A[i * N + k] * B[k * N + j];
                            }
                            C[i * N + j] = sum;
                        }
                    }
                }
            }
        }
        
        /* Verify result (simple checksum) */
        int checksum = 0;
        for (int i = 0; i < N * N; i += 64) {
            checksum ^= C[i];
        }
        printf("Matrix checksum: %d\n", checksum);
    }
    
    free(A);
    free(B);
    free(C);
    
    return 0;
}
