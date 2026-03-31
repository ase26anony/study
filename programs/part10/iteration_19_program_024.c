/* test_cache_coverage.c - Cover GCC i386 driver cache detection cases */
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

/* Cache-thrashing benchmark function */
static void benchmark_cache(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    const int elem_size = sizeof(int);
    const int num_elems = (buffer_size_kb * 1024) / elem_size;
    
    /* Allocate buffer larger than expected cache */
    int *buffer = (int*)malloc(num_elems * elem_size);
    if (!buffer) return;
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (int i = 0; i < num_elems; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    /* Cache-thrashing access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        uint32_t idx = iter;
        for (int i = 0; i < num_elems * 4; i++) {
            idx = (idx * 1103515245 + 12345) % num_elems;
            buffer[idx] = buffer[idx] * 3 + 1;
            MEMORY_BARRIER();
        }
    }
    
    /* Compute checksum to prevent elimination */
    for (int i = 0; i < num_elems; i += 64) {
        result ^= buffer[i];
    }
    
    /* Use result to prevent dead code elimination */
    if (result == 0x12345678) {
        printf("Impossible\n");
    }
    
    free(buffer);
}

/* Multi-versioned benchmark for different cache sizes */
#ifdef USE_MULTI_VERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
void run_all_benchmarks() {
    /* Test different buffer sizes to trigger various cache optimizations */
    benchmark_cache(100, 8);   /* L1 cache sizes */
    benchmark_cache(50, 16);
    benchmark_cache(30, 32);
    benchmark_cache(20, 64);
    benchmark_cache(10, 128);  /* Small L2 */
    benchmark_cache(5, 256);   /* Medium L2 */
    benchmark_cache(3, 512);   /* Large L2 */
    benchmark_cache(2, 1024);  /* Very large L2 */
    benchmark_cache(1, 2048);
    benchmark_cache(1, 4096);
    benchmark_cache(1, 6144);
}

/* Matrix multiplication to stress cache hierarchy */
static void matrix_multiply(int n) {
    volatile double *A, *B, *C;
    A = (double*)malloc(n * n * sizeof(double));
    B = (double*)malloc(n * n * sizeof(double));
    C = (double*)malloc(n * n * sizeof(double));
    
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < n * n; i++) {
        A[i] = i * 0.1;
        B[i] = i * 0.2;
        C[i] = 0.0;
    }
    
    /* Blocked matrix multiplication - cache aware */
    const int BLOCK_SIZE = 32;
    for (int i0 = 0; i0 < n; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < n; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < n; k0 += BLOCK_SIZE) {
                int i_end = (i0 + BLOCK_SIZE) < n ? (i0 + BLOCK_SIZE) : n;
                int j_end = (j0 + BLOCK_SIZE) < n ? (j0 + BLOCK_SIZE) : n;
                int k_end = (k0 + BLOCK_SIZE) < n ? (k0 + BLOCK_SIZE) : n;
                
                for (int i = i0; i < i_end; i++) {
                    for (int j = j0; j < j_end; j++) {
                        double sum = C[i * n + j];
                        for (int k = k0; k < k_end; k++) {
                            sum += A[i * n + k] * B[k * n + j];
                        }
                        C[i * n + j] = sum;
                    }
                }
            }
        }
    }
    
    /* Use result */
    volatile double check = 0.0;
    for (int i = 0; i < n; i++) {
        check += C[i * n + i];
    }
    
    if (check == 123456.0) {
        printf("Impossible\n");
    }
    
    free((void*)A); free((void*)B); free((void*)C);
}

/* Different test patterns for different CPU targets */
#if defined(TEST_CASE_0x0A) || defined(TEST_CASE_0x0C) || defined(TEST_CASE_0x0D) || defined(TEST_CASE_0x0E)
/* Small L1 cache patterns for Pentium III class CPUs */
__attribute__((target("arch=pentium3")))
void test_small_l1_cache() {
    /* These CPUs have small L1 caches (8-24KB) */
    benchmark_cache(200, 4);
    benchmark_cache(150, 8);
    benchmark_cache(100, 16);
    benchmark_cache(75, 24);
    matrix_multiply(128);  /* Fits in small L2 */
}
#endif

#if defined(TEST_CASE_0x21) || defined(TEST_CASE_0x24) || defined(TEST_CASE_0x39) || defined(TEST_CASE_0x3A) || \
    defined(TEST_CASE_0x3B) || defined(TEST_CASE_0x3C) || defined(TEST_CASE_0x3D) || defined(TEST_CASE_0x3E)
/* Pentium 4 class L2 cache patterns */
__attribute__((target("arch=pentium4")))
void test_p4_l2_cache() {
    /* Pentium 4 has larger L2 caches (128-2048KB) */
    benchmark_cache(100, 128);
    benchmark_cache(75, 256);
    benchmark_cache(50, 512);
    benchmark_cache(25, 1024);
    benchmark_cache(15, 2048);
    matrix_multiply(256);  /* Stress L2 cache */
}
#endif

#if defined(TEST_CASE_0x49) && !defined(XEON_MP)
/* Special case 0x49 for non-Xeon-MP CPUs (e.g., Xeon DP, some Pentium 4) */
__attribute__((target("arch=nocona")))
void test_case_0x49() {
    /* 4096KB L2 cache */
    benchmark_cache(20, 1024);
    benchmark_cache(10, 2048);
    benchmark_cache(5, 4096);
    matrix_multiply(512);
}
#endif

#if defined(TEST_CASE_0x48) || defined(TEST_CASE_0x4E)
/* Core 2 class with very large L2 caches */
__attribute__((target("arch=core2")))
void test_large_l2_cache() {
    /* Very large L2 caches (3072-6144KB) */
    benchmark_cache(10, 3072);
    benchmark_cache(5, 4096);
    benchmark_cache(3, 6144);
    matrix_multiply(768);
}
#endif

#if defined(TEST_CASE_0x60) || defined(TEST_CASE_0x66) || defined(TEST_CASE_0x67) || defined(TEST_CASE_0x68)
/* Enhanced L1 cache patterns */
__attribute__((target("arch=nocona")))
void test_enhanced_l1_cache() {
    /* Larger L1 caches (8-32KB) with different associativity */
    benchmark_cache(300, 8);
    benchmark_cache(200, 16);
    benchmark_cache(150, 32);
    matrix_multiply(192);
}
#endif

#if defined(TEST_CASE_0x78) || defined(TEST_CASE_0x79) || defined(TEST_CASE_0x7A) || defined(TEST_CASE_0x7B) || \
    defined(TEST_CASE_0x7C) || defined(TEST_CASE_0x7D) || defined(TEST_CASE_0x7F) || defined(TEST_CASE_0x80) || \
    defined(TEST_CASE_0x82) || defined(TEST_CASE_0x83) || defined(TEST_CASE_0x84) || defined(TEST_CASE_0x85) || \
    defined(TEST_CASE_0x86) || defined(TEST_CASE_0x87)
/* AMD K8/K10 cache patterns */
__attribute__((target("arch=k8")))
void test_amd_cache() {
    /* AMD-specific cache configurations */
    benchmark_cache(100, 128);
    benchmark_cache(75, 256);
    benchmark_cache(50, 512);
    benchmark_cache(25, 1024);
    benchmark_cache(15, 2048);
    matrix_multiply(384);
}
#endif

int main() {
    printf("Cache detection coverage test\n");
    
    /* Run generic benchmark that should trigger cache detection */
    run_all_benchmarks();
    
    /* Run architecture-specific tests based on compile-time defines */
#if defined(TEST_CASE_0x0A) || defined(TEST_CASE_0x0C) || defined(TEST_CASE_0x0D) || defined(TEST_CASE_0x0E)
    test_small_l1_cache();
#endif

#if defined(TEST_CASE_0x21) || defined(TEST_CASE_0x24) || defined(TEST_CASE_0x39) || defined(TEST_CASE_0x3A) || \
    defined(TEST_CASE_0x3B) || defined(TEST_CASE_0x3C) || defined(TEST_CASE_0x3D) || defined(TEST_CASE_0x3E)
    test_p4_l2_cache();
#endif

#if defined(TEST_CASE_0x49) && !defined(XEON_MP)
    test_case_0x49();
#endif

#if defined(TEST_CASE_0x48) || defined(TEST_CASE_0x4E)
    test_large_l2_cache();
#endif

#if defined(TEST_CASE_0x60) || defined(TEST_CASE_0x66) || defined(TEST_CASE_0x67) || defined(TEST_CASE_0x68)
    test_enhanced_l1_cache();
#endif

#if defined(TEST_CASE_0x78) || defined(TEST_CASE_0x79) || defined(TEST_CASE_0x7A) || defined(TEST_CASE_0x7B) || \
    defined(TEST_CASE_0x7C) || defined(TEST_CASE_0x7D) || defined(TEST_CASE_0x7F) || defined(TEST_CASE_0x80) || \
    defined(TEST_CASE_0x82) || defined(TEST_CASE_0x83) || defined(TEST_CASE_0x84) || defined(TEST_CASE_0x85) || \
    defined(TEST_CASE_0x86) || defined(TEST_CASE_0x87)
    test_amd_cache();
#endif
    
    /* Additional memory-intensive operation */
    matrix_multiply(256);
    
    printf("Test completed\n");
    return 0;
}
