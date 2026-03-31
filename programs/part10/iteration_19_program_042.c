/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
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
void benchmark_cache(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j;
    
    /* Allocate buffer larger than L2 cache to ensure cache thrashing */
    int elements = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    unsigned int seed = 123456789;
    for (i = 0; i < elements; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        buffer[i] = (int)seed;
    }
    
    /* Cache thrashing benchmark */
    for (j = 0; j < iterations; j++) {
        /* Access pattern designed to stress different cache configurations */
        int stride = 17; /* Prime number to avoid power-of-two alignment */
        for (i = 0; i < elements; i += stride) {
            buffer[(i * stride) % elements] += buffer[i] + j;
        }
        
        /* Another pattern with different stride */
        stride = 13;
        for (i = elements - 1; i >= 0; i -= stride) {
            buffer[i] ^= buffer[(i * 3) % elements];
        }
        
        COMPILER_BARRIER();
    }
    
    /* Compute final result to prevent optimization */
    for (i = 0; i < elements; i += 128) { /* Sample every 128th element */
        result ^= buffer[i];
    }
    
    /* Use result to prevent dead code elimination */
    if (result == 0xdeadbeef) {
        printf("Impossible condition\n");
    }
    
    free(buffer);
}

/* Multi-version function using target_clones for GCC */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
void multi_version_benchmark() {
    /* Different buffer sizes to potentially trigger different cache logic */
    benchmark_cache(100, 2048);  /* 2MB - larger than most L2 caches */
    benchmark_cache(50, 1024);   /* 1MB */
    benchmark_cache(200, 512);   /* 512KB */
    benchmark_cache(300, 256);   /* 256KB */
}

/* Special test for case 0x49 (Xeon MP vs non-Xeon MP) */
#ifdef TEST_XEON_MP
__attribute__((target("arch=pentium4")))
void test_xeon_mp_case() {
    /* This should trigger case 0x49 with xeon_mp flag potentially set */
    volatile int *huge_buffer = (volatile int*)malloc(16 * 1024 * 1024); /* 16MB */
    if (huge_buffer) {
        /* Access pattern that spans large memory area */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            huge_buffer[i] = i;
        }
        
        /* Complex access to force consideration of large cache */
        for (int iter = 0; iter < 100; iter++) {
            for (int i = 0; i < 1024 * 1024; i += 128) {
                huge_buffer[i] += huge_buffer[(i * 7) % (1024 * 1024)];
            }
            COMPILER_BARRIER();
        }
        
        free((void*)huge_buffer);
    }
}
#endif

int main(int argc, char *argv[]) {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache detection test for GCC i386 driver\n");
    printf("=========================================\n");
    
    start = clock();
    
    /* Run benchmarks with different characteristics */
    
    /* Test 1: Small working set (fits in L1) */
    printf("Test 1: Small working set (L1 cache)\n");
    benchmark_cache(1000, 32);  /* 32KB - typical L1 size */
    
    /* Test 2: Medium working set (fits in L2) */
    printf("Test 2: Medium working set (L2 cache)\n");
    benchmark_cache(500, 512);  /* 512KB - typical L2 size */
    
    /* Test 3: Large working set (exceeds L2) */
    printf("Test 3: Large working set (exceeds L2)\n");
    benchmark_cache(100, 4096); /* 4MB - larger than most L2 caches */
    
#ifdef USE_MULTIVERSIONING
    printf("Test 4: Multi-version benchmark\n");
    multi_version_benchmark();
#endif
    
#ifdef TEST_XEON_MP
    printf("Test 5: Xeon MP special case\n");
    test_xeon_mp_case();
#endif
    
    /* Additional stress test with matrix multiplication */
    printf("Test 6: Matrix multiplication stress test\n");
    {
        const int N = 256;
        volatile int *A = (volatile int*)malloc(N * N * sizeof(int));
        volatile int *B = (volatile int*)malloc(N * N * sizeof(int));
        volatile int *C = (volatile int*)malloc(N * N * sizeof(int));
        
        if (A && B && C) {
            /* Initialize matrices */
            for (int i = 0; i < N * N; i++) {
                A[i] = i % 100;
                B[i] = (i * 3) % 100;
                C[i] = 0;
            }
            
            /* Blocked matrix multiplication to stress cache */
            const int BLOCK = 32;
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
            COMPILER_BARRIER();
            
            free((void*)A);
            free((void*)B);
            free((void*)C);
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("\nTotal execution time: %.2f seconds\n", cpu_time_used);
    printf("Test completed.\n");
    
    return 0;
}
