/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barriers to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
#define MEMORY_BARRIER() __sync_synchronize()

/* Simple pseudo-random number generator for cache thrashing */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(uint32_t max) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return (lcg_state >> 16) % max;
}

/* Benchmark function template */
static void cache_thrash_benchmark(int *buffer, size_t size_elements, 
                                   int iterations, int stride) {
    volatile int sink = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < size_elements; i += stride) {
            buffer[i] = buffer[(i + stride) % size_elements] + 1;
        }
        COMPILER_BARRIER();
    }
    
    /* Prevent dead code elimination */
    for (size_t i = 0; i < size_elements; i += 1024) {
        sink ^= buffer[i];
    }
}

/* Architecture-specific benchmark variants using target attributes */
#ifdef TEST_PENTIUM3
__attribute__((target("arch=pentium3,tune=pentium3")))
#endif
static void benchmark_pentium3(void) {
    printf("Running Pentium III targeted benchmark...\n");
    
    /* Target L1: 16KB, L2: 256KB/512KB (cases 0x0c, 0x0d, 0x21) */
    size_t l1_size = 16 * 1024 / sizeof(int);  /* ~16KB */
    size_t l2_size = 512 * 1024 / sizeof(int); /* ~512KB */
    
    int *buffer1 = malloc(l2_size * 2 * sizeof(int));
    int *buffer2 = malloc(l2_size * 2 * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pattern */
    for (size_t i = 0; i < l2_size * 2; i++) {
        buffer1[i] = i;
        buffer2[i] = ~i;
    }
    
    /* Thrash through different cache levels */
    cache_thrash_benchmark(buffer1, l1_size, 1000, 7);
    cache_thrash_benchmark(buffer2, l2_size, 500, 13);
    
    /* Cross-buffer access to stress associativity */
    for (int i = 0; i < 10000; i++) {
        size_t idx = lcg_rand(l2_size * 2);
        buffer1[idx] = buffer2[idx] + buffer1[(idx + 64) % (l2_size * 2)];
    }
    
    MEMORY_BARRIER();
    
    free(buffer1);
    free(buffer2);
}

#ifdef TEST_PENTIUM4
__attribute__((target("arch=pentium4,tune=pentium4")))
#endif
static void benchmark_pentium4(void) {
    printf("Running Pentium 4 targeted benchmark...\n");
    
    /* Target L1: 8KB, L2: 256KB-2MB (cases 0x0a, 0x41-0x45, 0x78-0x87) */
    size_t l1_size = 8 * 1024 / sizeof(int);    /* ~8KB */
    size_t l2_size = 2048 * 1024 / sizeof(int); /* ~2MB */
    
    int *buffer = malloc(l2_size * sizeof(int) * 3);
    
    if (!buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Large stride to defeat prefetching and stress cache */
    for (int iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < l2_size * 3; i += 128) {
            buffer[i] = buffer[(i + 1024) % (l2_size * 3)] * 3 + 1;
        }
        COMPILER_BARRIER();
    }
    
    /* Random access pattern */
    for (int i = 0; i < 50000; i++) {
        size_t idx1 = lcg_rand(l2_size * 3);
        size_t idx2 = lcg_rand(l2_size * 3);
        buffer[idx1] = buffer[idx2] ^ 0x55555555;
    }
    
    MEMORY_BARRIER();
    
    free(buffer);
}

#ifdef TEST_XEON_MP
__attribute__((target("arch=nocona,tune=nocona")))
#endif
static void benchmark_xeon_mp(void) {
    printf("Running Xeon MP targeted benchmark...\n");
    
    /* Target large L2/L3 caches (cases 0x49 with xeon_mp=true, 0x4e) */
    size_t l3_size = 8192 * 1024 / sizeof(int); /* ~8MB */
    
    int *buffer = malloc(l3_size * sizeof(int));
    
    if (!buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Streaming write pattern */
    for (size_t i = 0; i < l3_size; i++) {
        buffer[i] = i * 3;
    }
    
    /* Reverse read pattern */
    volatile int sum = 0;
    for (size_t i = l3_size - 1; i > 0; i--) {
        sum += buffer[i];
    }
    
    /* Matrix-style access pattern */
    const int rows = 1024;
    const int cols = 1024;
    int *matrix = malloc(rows * cols * sizeof(int));
    
    if (matrix) {
        /* Column-major access to stress cache */
        for (int c = 0; c < cols; c++) {
            for (int r = 0; r < rows; r++) {
                matrix[r * cols + c] = r * c;
            }
        }
        
        /* Transpose operation */
        for (int i = 0; i < rows; i++) {
            for (int j = i + 1; j < cols; j++) {
                int temp = matrix[i * cols + j];
                matrix[i * cols + j] = matrix[j * cols + i];
                matrix[j * cols + i] = temp;
            }
        }
        
        free(matrix);
    }
    
    MEMORY_BARRIER();
    free(buffer);
}

#ifdef TEST_ATHLON64
__attribute__((target("arch=k8,tune=k8")))
#endif
static void benchmark_athlon64(void) {
    printf("Running Athlon64/K8 targeted benchmark...\n");
    
    /* Target L1: 64KB, L2: 512KB-1MB (cases 0x60, 0x66-0x68, 0x78-0x87) */
    size_t l1_size = 64 * 1024 / sizeof(int);   /* ~64KB */
    size_t l2_size = 1024 * 1024 / sizeof(int); /* ~1MB */
    
    /* Multiple buffers for associativity testing */
    int *buffers[8];
    for (int i = 0; i < 8; i++) {
        buffers[i] = malloc(l1_size * sizeof(int));
        if (buffers[i]) {
            memset(buffers[i], i, l1_size * sizeof(int));
        }
    }
    
    /* Interleave access across buffers */
    for (int iter = 0; iter < 1000; iter++) {
        for (int buf = 0; buf < 8; buf++) {
            if (buffers[buf]) {
                for (size_t i = 0; i < l1_size; i += 16) {
                    buffers[buf][i] = buffers[(buf + 1) % 8][i] + iter;
                }
            }
        }
        COMPILER_BARRIER();
    }
    
    /* Large working set for L2 */
    int *large_buffer = malloc(l2_size * 2 * sizeof(int));
    if (large_buffer) {
        /* Strided access pattern */
        for (int stride = 64; stride <= 4096; stride *= 2) {
            for (size_t i = 0; i < l2_size * 2; i += stride) {
                large_buffer[i] = large_buffer[(i + stride) % (l2_size * 2)] * 7;
            }
        }
        free(large_buffer);
    }
    
    for (int i = 0; i < 8; i++) {
        free(buffers[i]);
    }
}

#ifdef TEST_CORE2
__attribute__((target("arch=core2,tune=core2")))
#endif
static void benchmark_core2(void) {
    printf("Running Core 2 targeted benchmark...\n");
    
    /* Target various cache configurations (cases 0x49 without xeon_mp, 0x78-0x87) */
    size_t l1_size = 32 * 1024 / sizeof(int);   /* ~32KB */
    size_t l2_size = 4096 * 1024 / sizeof(int); /* ~4MB */
    
    /* Allocate buffers with different alignments */
    int *aligned_buffers[16];
    for (int i = 0; i < 16; i++) {
        posix_memalign((void**)&aligned_buffers[i], 64, l1_size * sizeof(int));
        if (aligned_buffers[i]) {
            for (size_t j = 0; j < l1_size; j++) {
                aligned_buffers[i][j] = (i * j) & 0xFF;
            }
        }
    }
    
    /* Complex access pattern with multiple streams */
    for (int phase = 0; phase < 100; phase++) {
        for (int buf = 0; buf < 16; buf++) {
            if (aligned_buffers[buf]) {
                /* Linear access */
                for (size_t i = 0; i < l1_size; i++) {
                    aligned_buffers[buf][i] += aligned_buffers[(buf + 1) % 16][i];
                }
                
                /* Skewed access */
                for (size_t i = 0; i < l1_size; i += 3) {
                    aligned_buffers[buf][i] *= 2;
                }
            }
        }
        COMPILER_BARRIER();
    }
    
    /* Large matrix multiplication kernel */
    const int N = 256;
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
        
        /* Blocked matrix multiplication */
        const int BLOCK = 32;
        for (int ii = 0; ii < N; ii += BLOCK) {
            for (int jj = 0; jj < N; jj += BLOCK) {
                for (int kk = 0; kk < N; kk += BLOCK) {
                    for (int i = ii; i < ii + BLOCK && i < N; i++) {
                        for (int j = jj; j < jj + BLOCK && j < N; j++) {
                            int sum = C[i * N + j];
                            for (int k = kk; k < kk + BLOCK && k < N; k++) {
                                sum += A[i * N + k] * B[k * N + j];
                            }
                            C[i * N + j] = sum;
                        }
                    }
                }
            }
        }
        
        free(A);
        free(B);
        free(C);
    }
    
    for (int i = 0; i < 16; i++) {
        free(aligned_buffers[i]);
    }
}

/* Generic benchmark that should work on any x86 */
static void benchmark_generic(void) {
    printf("Running generic x86 benchmark...\n");
    
    /* Use various buffer sizes to trigger different cache detection paths */
    size_t sizes[] = {
        8 * 1024 / sizeof(int),     /* 8KB - case 0x0a */
        16 * 1024 / sizeof(int),    /* 16KB - cases 0x0c, 0x0d */
        32 * 1024 / sizeof(int),    /* 32KB - case 0x2c */
        64 * 1024 / sizeof(int),    /* 64KB */
        128 * 1024 / sizeof(int),   /* 128KB - cases 0x39, 0x3b, 0x41, 0x79 */
        256 * 1024 / sizeof(int),   /* 256KB - cases 0x21, 0x3c, 0x42, 0x7a, 0x82 */
        512 * 1024 / sizeof(int),   /* 512KB - cases 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
        1024 * 1024 / sizeof(int),  /* 1MB - cases 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
        2048 * 1024 / sizeof(int),  /* 2MB - cases 0x45, 0x7d, 0x85 */
        4096 * 1024 / sizeof(int),  /* 4MB - case 0x49 */
        6144 * 1024 / sizeof(int),  /* 6MB - case 0x4e */
        8192 * 1024 / sizeof(int)   /* 8MB */
    };
    
    volatile int total = 0;
    
    for (size_t s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        size_t size = sizes[s];
        int *buffer = malloc(size * sizeof(int));
        
        if (!buffer) continue;
        
        /* Initialize */
        for (size_t i = 0; i < size; i++) {
            buffer[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Access with prime stride to avoid simple patterns */
        int stride = 9973; /* Prime */
        for (size_t i = 0; i < size; i++) {
            size_t idx = (i * stride) % size;
            buffer[idx] = buffer[(idx + 1) % size] + buffer[(idx + stride) % size];
        }
        
        /* Sum to prevent elimination */
        for (size_t i = 0; i < size; i += 97) {
            total ^= buffer[i];
        }
        
        free(buffer);
        COMPILER_BARRIER();
    }
    
    printf("Generic benchmark result: %d\n", total);
}

int main(int argc, char **argv) {
    printf("Cache Detection Coverage Test\n");
    printf("=============================\n");
    
    /* Run all architecture-specific benchmarks */
#ifdef TEST_PENTIUM3
    benchmark_pentium3();
#endif
    
#ifdef TEST_PENTIUM4
    benchmark_pentium4();
#endif
    
#ifdef TEST_XEON_MP
    benchmark_xeon_mp();
#endif
    
#ifdef TEST_ATHLON64
    benchmark_athlon64();
#endif
    
#ifdef TEST_CORE2
    benchmark_core2();
#endif
    
    /* Always run generic benchmark */
    benchmark_generic();
    
    printf("\nBenchmark completed.\n");
    
    /* Final memory operation to ensure all code is used */
    volatile int final_check = 0;
    int *final_buffer = malloc(1024 * sizeof(int));
    if (final_buffer) {
        for (int i = 0; i < 1024; i++) {
            final_buffer[i] = i;
            final_check += final_buffer[i];
        }
        free(final_buffer);
    }
    
    return final_check != 0;
}
