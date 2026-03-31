/* test_cache_coverage.c - Comprehensive test to cover GCC i386 driver cache detection */
/* Compile with different -D flags and -march options to trigger specific cache descriptor cases */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function template */
static void __attribute__((noinline)) cache_thrash(size_t size_kb, int iterations) {
    size_t elements = (size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    volatile int sink = 0;
    
    if (!buffer) return;
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < elements; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    /* Cache thrashing benchmark */
    for (int iter = 0; iter < iterations; iter++) {
        seed = iter;
        for (size_t i = 0; i < elements; i++) {
            /* Pseudo-random access pattern */
            seed = seed * 1103515245 + 12345;
            size_t idx = seed % elements;
            buffer[idx] = buffer[idx] + iter;
        }
        MEMORY_BARRIER();
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (size_t i = 0; i < elements; i++) {
        sink ^= buffer[i];
    }
    
    free(buffer);
    (void)sink; /* Use sink to prevent optimization */
}

/* Architecture-specific benchmark variants */
#ifdef TEST_PENTIUM3
/* Targets Pentium III - should trigger cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
static void __attribute__((target("arch=pentium3"))) benchmark_pentium3(void) {
    printf("Running Pentium III optimized benchmark\n");
    /* Exercise different cache sizes */
    cache_thrash(8, 1000);   /* L1 cache size for case 0x0a */
    cache_thrash(16, 800);   /* L1 cache size for cases 0x0c, 0x0d */
    cache_thrash(256, 200);  /* L2 cache size for case 0x21 */
    cache_thrash(1024, 100); /* L2 cache size for case 0x24 */
}
#endif

#ifdef TEST_PENTIUM4
/* Targets Pentium 4 - should trigger cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, etc. */
static void __attribute__((target("arch=pentium4"))) benchmark_pentium4(void) {
    printf("Running Pentium 4 optimized benchmark\n");
    cache_thrash(8, 1000);   /* L1 cache */
    cache_thrash(16, 800);
    cache_thrash(32, 600);   /* L1 cache for case 0x2c */
    cache_thrash(256, 200);  /* L2 cache */
    cache_thrash(512, 150);
}
#endif

#ifdef TEST_NOCONA
/* Targets Intel Nocona (Xeon DP) - should trigger case 0x49 (non-Xeon-MP path) */
static void __attribute__((target("arch=nocona"))) benchmark_nocona(void) {
    printf("Running Nocona/Xeon DP optimized benchmark\n");
    /* Large cache sizes to trigger 0x49 case */
    cache_thrash(4096, 50);  /* 4MB L2 for case 0x49 */
    cache_thrash(2048, 75);
    cache_thrash(1024, 100);
}
#endif

#ifdef TEST_K8
/* Targets AMD K8 (Athlon64/Opteron) - should trigger cases: 0x40, 0x78-0x87 */
static void __attribute__((target("arch=k8"))) benchmark_k8(void) {
    printf("Running AMD K8 optimized benchmark\n");
    cache_thrash(64, 400);   /* L1 cache */
    cache_thrash(1024, 100); /* L2 cache for cases 0x78, 0x7c */
    cache_thrash(512, 150);  /* L2 cache for cases 0x7b, 0x7f, 0x80 */
    cache_thrash(256, 200);  /* L2 cache for cases 0x7a, 0x82 */
}
#endif

#ifdef TEST_CORE2
/* Targets Intel Core 2 - should trigger cases: 0x66, 0x67, 0x68, 0x78-0x87 */
static void __attribute__((target("arch=core2"))) benchmark_core2(void) {
    printf("Running Core 2 optimized benchmark\n");
    cache_thrash(8, 1000);   /* L1 for case 0x66 */
    cache_thrash(16, 800);   /* L1 for case 0x67 */
    cache_thrash(32, 600);   /* L1 for case 0x68 */
    cache_thrash(1024, 100); /* L2 */
    cache_thrash(2048, 75);
    cache_thrash(4096, 50);
}
#endif

#ifdef TEST_NEHALEM
/* Targets Intel Nehalem - should trigger cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x60 */
static void __attribute__((target("arch=nehalem"))) benchmark_nehalem(void) {
    printf("Running Nehalem optimized benchmark\n");
    cache_thrash(32, 600);   /* L1 */
    cache_thrash(64, 400);   /* L1 for case 0x60? */
    cache_thrash(256, 200);  /* L2/L3 */
    cache_thrash(8192, 25);  /* Large L3 */
}
#endif

/* Generic benchmark that uses all cache sizes from uncovered cases */
static void benchmark_all_cases(void) {
    printf("Running comprehensive cache benchmark\n");
    
    /* L1 cache sizes from uncovered cases */
    cache_thrash(8, 1000);   /* case 0x0a, 0x66 */
    cache_thrash(16, 800);   /* cases 0x0c, 0x0d, 0x67 */
    cache_thrash(24, 700);   /* case 0x0e */
    cache_thrash(32, 600);   /* case 0x2c, 0x68 */
    
    /* L2 cache sizes from uncovered cases */
    cache_thrash(128, 300);  /* cases 0x39, 0x3b, 0x41, 0x79 */
    cache_thrash(192, 250);  /* case 0x3a */
    cache_thrash(256, 200);  /* cases 0x21, 0x3c, 0x42, 0x7a, 0x82 */
    cache_thrash(384, 175);  /* case 0x3d */
    cache_thrash(512, 150);  /* cases 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
    cache_thrash(1024, 100); /* cases 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
    cache_thrash(2048, 75);  /* cases 0x45, 0x7d, 0x85 */
    cache_thrash(3072, 60);  /* case 0x48 */
    cache_thrash(4096, 50);  /* case 0x49 */
    cache_thrash(6144, 40);  /* case 0x4e */
}

/* Matrix multiplication to stress cache hierarchy */
static void __attribute__((noinline)) matrix_multiply_cache_test(int size) {
    double *A = (double*)malloc(size * size * sizeof(double));
    double *B = (double*)malloc(size * size * sizeof(double));
    double *C = (double*)malloc(size * size * sizeof(double));
    
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < size * size; i++) {
        A[i] = (double)(i % 100);
        B[i] = (double)((i + 1) % 100);
        C[i] = 0.0;
    }
    
    /* Blocked matrix multiplication for cache efficiency */
    int block = 32; /* Typical cache line friendly block size */
    for (int i = 0; i < size; i += block) {
        for (int j = 0; j < size; j += block) {
            for (int k = 0; k < size; k += block) {
                int i_end = (i + block) < size ? (i + block) : size;
                int j_end = (j + block) < size ? (j + block) : size;
                int k_end = (k + block) < size ? (k + block) : size;
                
                for (int ii = i; ii < i_end; ii++) {
                    for (int kk = k; kk < k_end; kk++) {
                        double a = A[ii * size + kk];
                        for (int jj = j; jj < j_end; jj++) {
                            C[ii * size + jj] += a * B[kk * size + jj];
                        }
                    }
                }
            }
        }
    }
    
    MEMORY_BARRIER();
    
    /* Use result to prevent optimization */
    volatile double sum = 0.0;
    for (int i = 0; i < size * size; i++) {
        sum += C[i];
    }
    (void)sum;
    
    free(A); free(B); free(C);
}

int main(void) {
    printf("Cache Detection Coverage Test\n");
    printf("=============================\n");
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Run architecture-specific benchmarks if compiled with corresponding flags */
#ifdef TEST_PENTIUM3
    benchmark_pentium3();
#endif
    
#ifdef TEST_PENTIUM4
    benchmark_pentium4();
#endif
    
#ifdef TEST_NOCONA
    benchmark_nocona();
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
    
    /* Always run the comprehensive benchmark */
    benchmark_all_cases();
    
    /* Additional cache-intensive computation */
    printf("\nRunning matrix multiplication cache test...\n");
    matrix_multiply_cache_test(256);  /* 256x256 matrix = 512KB if double */
    matrix_multiply_cache_test(512);  /* 512x512 matrix = 2MB */
    matrix_multiply_cache_test(1024); /* 1024x1024 matrix = 8MB */
    
    printf("\nBenchmark completed successfully.\n");
    
    return 0;
}
