/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our benchmarks */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to exercise various cache descriptor cases */
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
        buffer1[i] = (int)(seed >> 16);
        buffer2[i] = (int)(seed >> 8);
    }
    
    COMPILER_BARRIER();
    
    /* Cache-thrashing memory access pattern */
    volatile int result = 0;
    for (int iter = 0; iter < iterations; iter++) {
        /* Strided access pattern to exercise different cache associativities */
        for (size_t i = 0; i < buffer_size; i += 64) {
            result += buffer1[i];
        }
        
        /* Reverse access pattern */
        for (size_t i = buffer_size - 1; i > 0; i -= 128) {
            result += buffer2[i];
        }
        
        /* Random-like but deterministic pattern using LCG */
        seed = iter * 0x1234567;
        for (int j = 0; j < 10000; j++) {
            seed = seed * 1664525 + 1013904223;
            size_t idx = seed % buffer_size;
            buffer1[idx] = result;
            result ^= buffer2[idx];
        }
        
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    printf("Benchmark result: %d\n", result);
    
    free((void*)buffer1);
    free((void*)buffer2);
}

/* Multi-versioned function using target_clones */
#ifdef USE_MULTI_VERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void multi_version_benchmark(int iterations) {
    cache_thrashing_benchmark(iterations);
}

/* Individual benchmark functions for specific cache descriptor cases */
#ifdef COVER_CASE_0x0A
/* 8KB L1, 2-way, 32B line - Pentium III */
__attribute__((target("arch=pentium3")))
static void bench_case_0x0a(void) {
    printf("Testing cache config 0x0a (8KB L1, 2-way, 32B)\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x0C
/* 16KB L1, 4-way, 32B line - Pentium III */
__attribute__((target("arch=pentium3")))
static void bench_case_0x0c(void) {
    printf("Testing cache config 0x0c (16KB L1, 4-way, 32B)\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x21
/* 256KB L2, 8-way, 64B line - Pentium III */
__attribute__((target("arch=pentium3")))
static void bench_case_0x21(void) {
    printf("Testing cache config 0x21 (256KB L2, 8-way, 64B)\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x2C
/* 32KB L1, 8-way, 64B line - Pentium 4 */
__attribute__((target("arch=pentium4")))
static void bench_case_0x2c(void) {
    printf("Testing cache config 0x2c (32KB L1, 8-way, 64B)\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x39
/* 128KB L2, 4-way, 64B line - Pentium 4 */
__attribute__((target("arch=pentium4")))
static void bench_case_0x39(void) {
    printf("Testing cache config 0x39 (128KB L2, 4-way, 64B)\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x49
/* 4096KB L2, 16-way, 64B line - Xeon DP (non-MP) */
__attribute__((target("arch=nocona")))
static void bench_case_0x49(void) {
    printf("Testing cache config 0x49 (4MB L2, 16-way, 64B) - non-Xeon-MP\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x60
/* 16KB L1, 8-way, 64B line - Intel Core/Core 2 */
__attribute__((target("arch=core2")))
static void bench_case_0x60(void) {
    printf("Testing cache config 0x60 (16KB L1, 8-way, 64B)\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x78
/* 1024KB L2, 4-way, 64B line - AMD K8 */
__attribute__((target("arch=k8")))
static void bench_case_0x78(void) {
    printf("Testing cache config 0x78 (1MB L2, 4-way, 64B)\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x82
/* 256KB L2, 8-way, 32B line */
__attribute__((target("arch=k8")))
static void bench_case_0x82(void) {
    printf("Testing cache config 0x82 (256KB L2, 8-way, 32B)\n");
    cache_thrashing_benchmark(100);
}
#endif

#ifdef COVER_CASE_0x86
/* 512KB L2, 4-way, 64B line */
__attribute__((target("arch=k8")))
static void bench_case_0x86(void) {
    printf("Testing cache config 0x86 (512KB L2, 4-way, 64B)\n");
    cache_thrashing_benchmark(100);
}
#endif

int main(int argc, char** argv) {
    int iterations = 10;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    printf("Cache Detection Coverage Test\n");
    printf("=============================\n");
    
    /* Run benchmarks for specific cases if compiled with corresponding defines */
#ifdef COVER_CASE_0x0A
    bench_case_0x0a();
#endif
#ifdef COVER_CASE_0x0C
    bench_case_0x0c();
#endif
#ifdef COVER_CASE_0x21
    bench_case_0x21();
#endif
#ifdef COVER_CASE_0x2C
    bench_case_0x2c();
#endif
#ifdef COVER_CASE_0x39
    bench_case_0x39();
#endif
#ifdef COVER_CASE_0x49
    bench_case_0x49();
#endif
#ifdef COVER_CASE_0x60
    bench_case_0x60();
#endif
#ifdef COVER_CASE_0x78
    bench_case_0x78();
#endif
#ifdef COVER_CASE_0x82
    bench_case_0x82();
#endif
#ifdef COVER_CASE_0x86
    bench_case_0x86();
#endif
    
    /* Run multi-version benchmark if enabled */
#ifdef USE_MULTI_VERSIONING
    printf("\nRunning multi-version benchmark:\n");
    multi_version_benchmark(iterations);
#else
    /* Run generic benchmark */
    printf("\nRunning generic benchmark:\n");
    cache_thrashing_benchmark(iterations);
#endif
    
    /* Additional test: Matrix multiplication to exercise cache blocking */
    printf("\nRunning matrix multiplication test:\n");
    const int N = 512;
    volatile double* A = (volatile double*)malloc(N * N * sizeof(double));
    volatile double* B = (volatile double*)malloc(N * N * sizeof(double));
    volatile double* C = (volatile double*)malloc(N * N * sizeof(double));
    
    if (A && B && C) {
        /* Simple matrix multiply with cache-unfriendly access pattern */
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                double sum = 0.0;
                for (int k = 0; k < N; k++) {
                    sum += A[i * N + k] * B[k * N + j];
                }
                C[i * N + j] = sum;
            }
        }
        
        /* Use result */
        volatile double check = 0.0;
        for (int i = 0; i < N * N; i += 128) {
            check += C[i];
        }
        printf("Matrix test check: %f\n", check);
    }
    
    free((void*)A);
    free((void*)B);
    free((void*)C);
    
    return 0;
}
