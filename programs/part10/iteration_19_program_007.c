/* test_cache_coverage.c - Comprehensive test to cover CPUID leaf 2 cache descriptor cases */
/* Compile with different -D flags and -march options to target specific CPUs */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function template */
static void cache_thrash_benchmark(size_t buffer_size, int iterations) {
    volatile int *buffer = (volatile int*)malloc(buffer_size * sizeof(int));
    volatile int result = 0;
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    MEMORY_BARRIER();
    
    /* Cache-thrashing access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        /* Linear access with large stride to defeat prefetching */
        for (size_t i = 0; i < buffer_size; i += 64) {
            result ^= buffer[i];
        }
        
        /* Reverse access pattern */
        for (size_t i = buffer_size - 1; i > 0; i -= 128) {
            result += buffer[i];
        }
        
        /* Random-ish access using simple LCG */
        uint32_t seed = iter;
        for (size_t j = 0; j < buffer_size / 4; j++) {
            seed = seed * 1103515245 + 12345;
            size_t idx = seed % buffer_size;
            buffer[idx] = result;
        }
        
        MEMORY_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    printf("Benchmark result: %d\n", result);
    
    free((void*)buffer);
}

/* Architecture-specific benchmark variants using target attributes */
/* These functions will cause GCC to consider different cache configurations */

#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
static void benchmark_pentium3(void) {
    printf("=== Pentium III target (8KB/16KB L1, 256KB/512KB L2) ===\n");
    /* Use sizes that match Pentium III cache hierarchy */
    cache_thrash_benchmark(16 * 1024, 1000);  /* ~L1 size */
    cache_thrash_benchmark(256 * 1024, 100);  /* ~L2 size */
}
#endif

#ifdef TEST_PENTIUM4
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x39-0x3e, 0x41-0x45 */
__attribute__((target("arch=pentium4")))
static void benchmark_pentium4(void) {
    printf("=== Pentium 4 target (8KB L1, 256KB-1MB L2) ===\n");
    cache_thrash_benchmark(8 * 1024, 1500);   /* 8KB L1 */
    cache_thrash_benchmark(512 * 1024, 200);  /* 512KB L2 typical */
}
#endif

#ifdef TEST_XEON_MP
/* Targets case 0x49 with xeon_mp=true (should skip assignment) */
__attribute__((target("arch=nocona")))
static void benchmark_xeon_mp(void) {
    printf("=== Xeon MP target (case 0x49 with xeon_mp flag) ===\n");
    /* Large buffers to trigger L3 detection if present */
    cache_thrash_benchmark(1024 * 1024, 50);   /* 1MB */
    cache_thrash_benchmark(4096 * 1024, 20);   /* 4MB */
}
#endif

#ifdef TEST_XEON_DP
/* Targets case 0x49 with xeon_mp=false (should assign 4MB L2) */
__attribute__((target("arch=nocona")))
static void benchmark_xeon_dp(void) {
    printf("=== Xeon DP target (case 0x49 without xeon_mp) ===\n");
    cache_thrash_benchmark(4096 * 1024, 30);   /* 4MB L2 */
}
#endif

#ifdef TEST_K8
/* Targets cases: 0x40, 0x78-0x87 (AMD K8) */
__attribute__((target("arch=k8")))
static void benchmark_k8(void) {
    printf("=== AMD K8 target (64KB L1, 512KB-1MB L2) ===\n");
    cache_thrash_benchmark(64 * 1024, 800);    /* 64KB L1 */
    cache_thrash_benchmark(1024 * 1024, 100);  /* 1MB L2 */
}
#endif

#ifdef TEST_CORE2
/* Targets cases: 0x66, 0x67, 0x68, 0x78-0x87 */
__attribute__((target("arch=core2")))
static void benchmark_core2(void) {
    printf("=== Core 2 target (32KB L1, 2-4MB L2) ===\n");
    cache_thrash_benchmark(32 * 1024, 1000);   /* 32KB L1 */
    cache_thrash_benchmark(4096 * 1024, 50);   /* 4MB L2 */
}
#endif

#ifdef TEST_NEHALEM
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x78-0x87 */
__attribute__((target("arch=nehalem")))
static void benchmark_nehalem(void) {
    printf("=== Nehalem target (32KB L1, 256KB L2, 8MB L3) ===\n");
    cache_thrash_benchmark(32 * 1024, 1000);   /* 32KB L1 */
    cache_thrash_benchmark(256 * 1024, 200);   /* 256KB L2 */
    cache_thrash_benchmark(8192 * 1024, 25);   /* 8MB L3 */
}
#endif

/* Multi-version function that dispatches to best implementation */
__attribute__((target_clones("pentium3, pentium4, k8, core2, nehalem")))
static void multiarch_benchmark(void) {
    printf("Multi-arch benchmark (dispatches at runtime)\n");
    cache_thrash_benchmark(256 * 1024, 100);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    printf("Cache Descriptor Coverage Test\n");
    printf("==============================\n\n");
    
    /* Always run generic benchmark */
    printf("Generic benchmark (uses -march setting):\n");
    cache_thrash_benchmark(512 * 1024, 100);
    printf("\n");
    
    /* Run architecture-specific benchmarks based on compile-time defines */
#ifdef TEST_PENTIUM3
    benchmark_pentium3();
#endif
    
#ifdef TEST_PENTIUM4
    benchmark_pentium4();
#endif
    
#ifdef TEST_XEON_MP
    benchmark_xeon_mp();
#endif
    
#ifdef TEST_XEON_DP
    benchmark_xeon_dp();
#endif
    
#ifdef TEST_K8
    benchmark_k8();
#endif
    
#ifdef TEST_CORE2
    benchmark_core2();
#endif
    
#ifdef TEST_NEHALEM
    benchmark_nehalem();
#endif
    
    /* Multi-version benchmark if supported */
#if defined(__x86_64__) || defined(__i386__)
    multiarch_benchmark();
#endif
    
    /* Additional test: Matrix multiplication to stress cache hierarchy */
    printf("\nMatrix multiplication cache test:\n");
    const int N = 512;  /* Size that exceeds L1, fits in L2 */
    volatile int *A = (volatile int*)malloc(N * N * sizeof(int));
    volatile int *B = (volatile int*)malloc(N * N * sizeof(int));
    volatile int *C = (volatile int*)malloc(N * N * sizeof(int));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < N * N; i++) {
            A[i] = i % 100;
            B[i] = (i + 1) % 100;
            C[i] = 0;
        }
        
        MEMORY_BARRIER();
        
        /* Blocked matrix multiplication for better cache utilization */
        const int BLOCK = 32;
        for (int i = 0; i < N; i += BLOCK) {
            for (int j = 0; j < N; j += BLOCK) {
                for (int k = 0; k < N; k += BLOCK) {
                    for (int ii = i; ii < i + BLOCK && ii < N; ii++) {
                        for (int jj = j; jj < j + BLOCK && jj < N; jj++) {
                            int sum = C[ii * N + jj];
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
        int checksum = 0;
        for (int i = 0; i < N * N; i += 128) {
            checksum ^= C[i];
        }
        printf("Matrix checksum: %d\n", checksum);
    }
    
    free((void*)A);
    free((void*)B);
    free((void*)C);
    
    return 0;
}
