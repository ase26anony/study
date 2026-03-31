/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
/* Compile with different -D flags and -march options to cover specific cases */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our benchmark */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function */
static void cache_thrash(size_t size_kb, int iterations) {
    size_t elements = (size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    volatile int result = 0;
    
    if (!buffer) return;
    
    /* Initialize with pseudo-random pattern */
    for (size_t i = 0; i < elements; i++) {
        buffer[i] = (int)(i * 1103515245ULL + 12345) & 0x7FFF;
    }
    
    /* Cache thrashing benchmark */
    for (int iter = 0; iter < iterations; iter++) {
        /* Access pattern designed to stress cache associativity */
        size_t stride = 17; /* Prime number to avoid simple patterns */
        for (size_t i = 0; i < elements; i++) {
            size_t idx = (i * stride) % elements;
            buffer[idx] = buffer[idx] * 3 + 1;
        }
        
        COMPILER_BARRIER();
        
        /* Another pattern with different stride */
        stride = 13;
        for (size_t i = 0; i < elements; i++) {
            size_t idx = (i * stride) % elements;
            result += buffer[idx];
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Cache thrash result: %d (size: %zu KB)\n", result, size_kb);
    
    free(buffer);
}

/* Function targeting specific CPU architectures using GCC attributes */
#ifdef TEST_PENTIUM3
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
__attribute__((target("arch=core2")))
#endif
static void architecture_specific_benchmark(void) {
    /* Test different cache sizes to trigger various detection paths */
    
    /* L1 cache sizes (8KB - 32KB) - triggers cases 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x60, 0x66-0x68 */
    cache_thrash(8, 1000);   /* May trigger case 0x0a or 0x66 */
    cache_thrash(16, 1000);  /* May trigger cases 0x0c, 0x0d, 0x67 */
    cache_thrash(24, 1000);  /* May trigger case 0x0e */
    cache_thrash(32, 1000);  /* May trigger cases 0x2c, 0x68 */
    
    COMPILER_BARRIER();
    
    /* L2 cache sizes (128KB - 2048KB) - triggers many L2 cases */
    cache_thrash(128, 500);   /* May trigger cases 0x39, 0x3b, 0x41, 0x79 */
    cache_thrash(192, 500);   /* May trigger case 0x3a */
    cache_thrash(256, 500);   /* May trigger cases 0x21, 0x3c, 0x42, 0x7a, 0x82 */
    cache_thrash(384, 500);   /* May trigger case 0x3d */
    cache_thrash(512, 500);   /* May trigger cases 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
    
    COMPILER_BARRIER();
    
    /* Larger L2/L3 cache sizes */
    cache_thrash(1024, 250);  /* May trigger cases 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
    cache_thrash(2048, 250);  /* May trigger cases 0x45, 0x7d, 0x85 */
    cache_thrash(3072, 100);  /* May trigger case 0x48 */
    cache_thrash(4096, 100);  /* May trigger case 0x49 (non-Xeon-MP) */
    cache_thrash(6144, 100);  /* May trigger case 0x4e */
}

/* Multi-versioned function using target_clones if available */
#if defined(__GNUC__) && __GNUC__ >= 6
__attribute__((target_clones("default,arch=pentium3,arch=pentium4,arch=nocona,arch=k8,arch=core2")))
#endif
static void multi_arch_benchmark(void) {
    /* This will generate multiple versions for different architectures */
    cache_thrash(256, 100);
    cache_thrash(1024, 50);
}

/* Main benchmark driver */
int main(void) {
    printf("Starting cache detection coverage test...\n");
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Run architecture-specific benchmark if compiled with specific target */
#if defined(TEST_PENTIUM3) || defined(TEST_PENTIUM4) || defined(TEST_NOCONA) || \
    defined(TEST_K8) || defined(TEST_CORE2)
    printf("Running architecture-specific benchmark...\n");
    architecture_specific_benchmark();
#endif
    
    /* Always run multi-arch benchmark if supported */
    printf("Running multi-architecture benchmark...\n");
    multi_arch_benchmark();
    
    /* Additional test: Matrix multiplication to stress cache hierarchy */
    printf("Running matrix multiplication test...\n");
    const int MATRIX_SIZE = 512;
    double *A = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *B = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *C = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                A[i * MATRIX_SIZE + j] = (double)rand() / RAND_MAX;
                B[i * MATRIX_SIZE + j] = (double)rand() / RAND_MAX;
                C[i * MATRIX_SIZE + j] = 0.0;
            }
        }
        
        /* Blocked matrix multiplication - cache aware */
        const int BLOCK_SIZE = 32;
        for (int ii = 0; ii < MATRIX_SIZE; ii += BLOCK_SIZE) {
            for (int jj = 0; jj < MATRIX_SIZE; jj += BLOCK_SIZE) {
                for (int kk = 0; kk < MATRIX_SIZE; kk += BLOCK_SIZE) {
                    for (int i = ii; i < ii + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                        for (int j = jj; j < jj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                            double sum = C[i * MATRIX_SIZE + j];
                            for (int k = kk; k < kk + BLOCK_SIZE && k < MATRIX_SIZE; k++) {
                                sum += A[i * MATRIX_SIZE + k] * B[k * MATRIX_SIZE + j];
                            }
                            C[i * MATRIX_SIZE + j] = sum;
                        }
                    }
                }
            }
        }
        
        /* Compute checksum */
        double checksum = 0.0;
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            checksum += C[i];
        }
        printf("Matrix multiplication checksum: %f\n", checksum);
    }
    
    free(A);
    free(B);
    free(C);
    
    printf("Cache detection coverage test completed.\n");
    return 0;
}
