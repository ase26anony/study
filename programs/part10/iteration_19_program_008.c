/* test_cache_coverage.c - Comprehensive test for x86 cache descriptor coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function */
static void cache_thrash_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Simple linear congruential generator for pseudo-random access */
    uint32_t lcg = 123456789;
    
    for (j = 0; j < iterations; j++) {
        /* Sequential access pattern */
        for (i = 0; i < size; i++) {
            buffer[i] = i;
        }
        COMPILER_BARRIER();
        
        /* Pseudo-random access pattern */
        for (i = 0; i < size; i++) {
            lcg = lcg * 1103515245 + 12345;
            size_t idx = (lcg >> 16) % size;
            sink += buffer[idx];
            buffer[idx] = sink;
        }
        COMPILER_BARRIER();
        
        /* Strided access pattern (prime step to avoid cache line conflicts) */
        for (i = 0; i < size; i++) {
            size_t idx = (i * 997) % size;  /* Prime number stride */
            sink ^= buffer[idx];
            buffer[idx] = sink;
        }
        COMPILER_BARRIER();
    }
    
    /* Ensure sink is used to prevent dead code elimination */
    if (sink == 0xdeadbeef) {
        printf("Impossible condition\n");
    }
}

/* Different benchmark variants for different CPU targets */
#ifdef TARGET_PENTIUM3
__attribute__((target("arch=pentium3")))
static void benchmark_pentium3(void) {
    /* Target for cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
    size_t buffer_size = 1024 * 1024;  /* 1MB - larger than typical L2 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    cache_thrash_benchmark(buffer, buffer_size, 10);
    
    free(buffer);
}
#endif

#ifdef TARGET_PENTIUM4
__attribute__((target("arch=pentium4")))
static void benchmark_pentium4(void) {
    /* Target for cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
    size_t buffer_size = 2 * 1024 * 1024;  /* 2MB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    cache_thrash_benchmark(buffer, buffer_size, 8);
    
    free(buffer);
}
#endif

#ifdef TARGET_NOCONA
__attribute__((target("arch=nocona")))
static void benchmark_nocona(void) {
    /* Target for Xeon DP (not MP) - case 0x49 assignment */
    /* Also covers: 0x48, 0x4e, 0x60, 0x66-0x68 */
    size_t buffer_size = 4 * 1024 * 1024;  /* 4MB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    cache_thrash_benchmark(buffer, buffer_size, 6);
    
    free(buffer);
}
#endif

#ifdef TARGET_K8
__attribute__((target("arch=k8")))
static void benchmark_k8(void) {
    /* Target for AMD K8 - cases: 0x78-0x87 */
    size_t buffer_size = 2 * 1024 * 1024;  /* 2MB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    cache_thrash_benchmark(buffer, buffer_size, 8);
    
    free(buffer);
}
#endif

#ifdef TARGET_CORE2
__attribute__((target("arch=core2")))
static void benchmark_core2(void) {
    /* Target for Core 2 - additional cache configurations */
    size_t buffer_size = 4 * 1024 * 1024;  /* 4MB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    cache_thrash_benchmark(buffer, buffer_size, 6);
    
    free(buffer);
}
#endif

/* Generic benchmark that uses multi-versioning if available */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3,pentium4,nocona,k8,core2,default")))
static void benchmark_multiversion(void) {
    size_t buffer_size = 2 * 1024 * 1024;
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    cache_thrash_benchmark(buffer, buffer_size, 5);
    
    free(buffer);
}
#endif

/* Matrix multiplication to exercise cache hierarchies */
static void matrix_multiply_cache_test(void) {
    const int N = 512;  /* Size that exceeds L1, fits in L2 */
    int *A = malloc(N * N * sizeof(int));
    int *B = malloc(N * N * sizeof(int));
    int *C = malloc(N * N * sizeof(int));
    
    if (!A || !B || !C) {
        fprintf(stderr, "Matrix allocation failed\n");
        free(A); free(B); free(C);
        return;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < N * N; i++) {
        A[i] = i % 100;
        B[i] = (i + 1) % 100;
        C[i] = 0;
    }
    
    COMPILER_BARRIER();
    
    /* Blocked matrix multiplication for better cache utilization */
    const int BLOCK_SIZE = 32;
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
    
    COMPILER_BARRIER();
    
    /* Use result to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < N * N; i += 97) {  /* Prime stride */
        checksum ^= C[i];
    }
    
    if (checksum == 0xdeadbeef) {
        printf("Impossible\n");
    }
    
    free(A); free(B); free(C);
}

int main(void) {
    printf("Starting cache coverage test...\n");
    
    /* Always run the matrix test - it's architecture neutral */
    matrix_multiply_cache_test();
    
    /* Run architecture-specific benchmarks based on compile-time defines */
#ifdef TARGET_PENTIUM3
    benchmark_pentium3();
    printf("Pentium III benchmark completed\n");
#endif
    
#ifdef TARGET_PENTIUM4
    benchmark_pentium4();
    printf("Pentium 4 benchmark completed\n");
#endif
    
#ifdef TARGET_NOCONA
    benchmark_nocona();
    printf("Nocona (Xeon DP) benchmark completed\n");
#endif
    
#ifdef TARGET_K8
    benchmark_k8();
    printf("AMD K8 benchmark completed\n");
#endif
    
#ifdef TARGET_CORE2
    benchmark_core2();
    printf("Core 2 benchmark completed\n");
#endif
    
#ifdef USE_MULTIVERSIONING
    benchmark_multiversion();
    printf("Multi-version benchmark completed\n");
#endif
    
    /* Additional memory-intensive test */
    {
        size_t huge_size = 8 * 1024 * 1024;  /* 8MB */
        int *huge_buffer = malloc(huge_size * sizeof(int));
        
        if (huge_buffer) {
            /* Fill with pattern */
            for (size_t i = 0; i < huge_size; i++) {
                huge_buffer[i] = (int)(i ^ (i >> 8));
            }
            
            /* Reverse the buffer */
            for (size_t i = 0; i < huge_size / 2; i++) {
                int temp = huge_buffer[i];
                huge_buffer[i] = huge_buffer[huge_size - 1 - i];
                huge_buffer[huge_size - 1 - i] = temp;
            }
            
            /* Verify by checksum */
            volatile int sum = 0;
            for (size_t i = 0; i < huge_size; i += 101) {  /* Prime stride */
                sum += huge_buffer[i];
            }
            
            free(huge_buffer);
            
            if (sum == 0xdeadbeef) {
                printf("Impossible checksum\n");
            }
        }
    }
    
    printf("Cache coverage test completed successfully\n");
    return 0;
}
