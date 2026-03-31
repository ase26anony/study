/* test_cache_coverage.c - Comprehensive test to cover GCC i386 driver cache detection cases */
/* Compile with different -D flags and -march options to target specific CPU cache configurations */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function template */
static void cache_thrash_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Simple linear congruential generator for pseudo-random access */
    uint32_t seed = 0xDEADBEEF;
    
    for (j = 0; j < iterations; j++) {
        for (i = 0; i < size; i++) {
            /* Pseudo-random index generation */
            seed = seed * 1103515245 + 12345;
            size_t idx = (seed >> 16) % size;
            
            /* Read-modify-write pattern */
            buffer[idx] = buffer[idx] * 3 + 1;
        }
        COMPILER_BARRIER();
    }
    
    /* Ensure all writes are completed and prevent dead code elimination */
    for (i = 0; i < size; i++) {
        sink ^= buffer[i];
    }
    COMPILER_BARRIER();
    
    /* Use sink to prevent optimization */
    if (sink == 0x12345678) {
        printf("Impossible condition\n");
    }
}

/* Function with target-specific attributes for different CPU architectures */
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
#ifdef TEST_NEHALEM
__attribute__((target("arch=nehalem")))
#endif
static void targeted_cache_benchmark(int arch_id) {
    /* Allocate buffers sized to exceed typical L2 caches */
    const size_t buffer_size = 4 * 1024 * 1024; /* 4MB */
    const int iterations = 100;
    
    int *buffer1 = (int*)malloc(buffer_size * sizeof(int));
    int *buffer2 = (int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize buffers with pattern */
    for (size_t i = 0; i < buffer_size; i++) {
        buffer1[i] = (int)(i ^ (i >> 8));
        buffer2[i] = (int)(i ^ (i >> 16));
    }
    
    COMPILER_BARRIER();
    
    /* Perform cache-thrashing operations */
    cache_thrash_benchmark(buffer1, buffer_size, iterations);
    cache_thrash_benchmark(buffer2, buffer_size, iterations);
    
    /* Cross-buffer operations to stress cache hierarchy */
    for (int iter = 0; iter < 50; iter++) {
        for (size_t i = 0; i < buffer_size; i += 64) { /* 64-byte stride */
            buffer2[i] = buffer1[i] + buffer2[(i + 256) % buffer_size];
        }
        COMPILER_BARRIER();
    }
    
    /* Final computation to prevent optimization */
    volatile int result = 0;
    for (size_t i = 0; i < buffer_size; i += 128) {
        result ^= buffer1[i] ^ buffer2[i];
    }
    
    printf("Architecture %d benchmark complete. Checksum: %d\n", arch_id, result);
    
    free(buffer1);
    free(buffer2);
}

/* Multi-versioned function using target_clones attribute */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3,pentium4,nocona,k8,core2,nehalem")))
#endif
static void multi_version_cache_test(void) {
    const size_t small_buf_size = 32 * 1024; /* 32KB - L1 cache sized */
    const size_t large_buf_size = 2 * 1024 * 1024; /* 2MB - L2/L3 sized */
    
    int *small_buf = (int*)malloc(small_buf_size * sizeof(int));
    int *large_buf = (int*)malloc(large_buf_size * sizeof(int));
    
    if (!small_buf || !large_buf) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with different patterns */
    for (size_t i = 0; i < small_buf_size; i++) {
        small_buf[i] = (int)(i * 3);
    }
    for (size_t i = 0; i < large_buf_size; i++) {
        large_buf[i] = (int)(i * 5);
    }
    
    /* Mixed access pattern to stress all cache levels */
    volatile int accumulator = 0;
    for (int rep = 0; rep < 1000; rep++) {
        /* Access small buffer (likely in L1) */
        for (size_t i = 0; i < small_buf_size; i += 8) {
            small_buf[i] = small_buf[i] * 2 + 1;
        }
        
        /* Access large buffer (likely in L2/L3 or RAM) */
        for (size_t i = 0; i < large_buf_size; i += 256) {
            large_buf[i] = large_buf[i] + small_buf[i % small_buf_size];
        }
        
        COMPILER_BARRIER();
        
        /* Periodic mixing */
        if (rep % 100 == 0) {
            for (size_t i = 0; i < small_buf_size; i += 16) {
                accumulator ^= small_buf[i];
            }
            for (size_t i = 0; i < large_buf_size; i += 1024) {
                accumulator ^= large_buf[i];
            }
        }
    }
    
    printf("Multi-version test accumulator: %d\n", accumulator);
    
    free(small_buf);
    free(large_buf);
}

/* Matrix multiplication to stress cache hierarchy */
#ifdef TEST_MATRIX
__attribute__((optimize("unroll-loops")))
#endif
static void matrix_cache_test(int size) {
    /* Use volatile to prevent aggressive optimization */
    volatile double *A = (double*)malloc(size * size * sizeof(double));
    volatile double *B = (double*)malloc(size * size * sizeof(double));
    volatile double *C = (double*)malloc(size * size * sizeof(double));
    
    if (!A || !B || !C) {
        fprintf(stderr, "Matrix allocation failed\n");
        exit(1);
    }
    
    /* Initialize matrices */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            A[i * size + j] = (double)((i + j) % 7);
            B[i * size + j] = (double)((i * j) % 11);
            C[i * size + j] = 0.0;
        }
    }
    
    COMPILER_BARRIER();
    
    /* Blocked matrix multiplication - cache aware */
    const int block_size = 32; /* Typical cache line friendly */
    for (int i0 = 0; i0 < size; i0 += block_size) {
        for (int j0 = 0; j0 < size; j0 += block_size) {
            for (int k0 = 0; k0 < size; k0 += block_size) {
                for (int i = i0; i < i0 + block_size && i < size; i++) {
                    for (int j = j0; j < j0 + block_size && j < size; j++) {
                        double sum = C[i * size + j];
                        for (int k = k0; k < k0 + block_size && k < size; k++) {
                            sum += A[i * size + k] * B[k * size + j];
                        }
                        C[i * size + j] = sum;
                    }
                }
            }
        }
    }
    
    COMPILER_BARRIER();
    
    /* Compute checksum */
    volatile double checksum = 0.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            checksum += C[i * size + j];
        }
    }
    
    printf("Matrix test checksum: %f\n", checksum);
    
    free((void*)A);
    free((void*)B);
    free((void*)C);
}

int main(int argc, char **argv) {
    printf("Cache detection coverage test\n");
    
    /* Run architecture-specific tests based on compilation flags */
    
#ifdef TEST_PENTIUM3
    /* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
    printf("Testing Pentium III cache configuration...\n");
    targeted_cache_benchmark(1);
    matrix_cache_test(256);
#endif

#ifdef TEST_PENTIUM4
    /* Targets cases: 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x41-0x45 */
    printf("Testing Pentium 4 cache configuration...\n");
    targeted_cache_benchmark(2);
    matrix_cache_test(512);
#endif

#ifdef TEST_NOCONA
    /* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66, 0x67, 0x68 */
    printf("Testing Nocona/Xeon DP cache configuration...\n");
    targeted_cache_benchmark(3);
    matrix_cache_test(1024);
#endif

#ifdef TEST_K8
    /* Targets cases: 0x48, 0x4e, 0x78-0x87 */
    printf("Testing AMD K8 cache configuration...\n");
    targeted_cache_benchmark(4);
    matrix_cache_test(512);
#endif

#ifdef TEST_CORE2
    /* Targets various L2 cache cases */
    printf("Testing Core 2 cache configuration...\n");
    targeted_cache_benchmark(5);
    matrix_cache_test(1024);
#endif

#ifdef TEST_NEHALEM
    /* Targets newer cache configurations */
    printf("Testing Nehalem cache configuration...\n");
    targeted_cache_benchmark(6);
    matrix_cache_test(1024);
#endif

#ifdef USE_MULTIVERSIONING
    /* Test all versions in one binary */
    printf("Testing multi-versioned function...\n");
    multi_version_cache_test();
#endif

    /* Always run a generic test */
    printf("Running generic cache test...\n");
    {
        const size_t test_size = 1 * 1024 * 1024; /* 1MB */
        int *test_buf = (int*)malloc(test_size * sizeof(int));
        
        if (test_buf) {
            for (size_t i = 0; i < test_size; i++) {
                test_buf[i] = (int)i;
            }
            
            cache_thrash_benchmark(test_buf, test_size, 10);
            
            volatile int sum = 0;
            for (size_t i = 0; i < test_size; i += 64) {
                sum += test_buf[i];
            }
            
            printf("Generic test sum: %d\n", sum);
            free(test_buf);
        }
    }
    
    return 0;
}
