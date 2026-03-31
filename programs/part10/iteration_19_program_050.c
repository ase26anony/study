/* test_cache_descriptors.c
 * 
 * This test program aims to cover specific cache descriptor cases in GCC's
 * i386 driver by targeting various x86 microarchitectures with different
 * cache configurations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Target-specific function declarations */
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
static void perform_cache_benchmark(int *buffer, size_t size);

/* Global volatile to prevent dead code elimination */
volatile int g_result = 0;

/* Simple pseudo-random number generator for memory access patterns */
static inline uint32_t lcg(uint32_t *state) {
    return *state = *state * 1103515245 + 12345;
}

/* Cache thrashing benchmark function */
static void perform_cache_benchmark(int *buffer, size_t size) {
    uint32_t seed = 0xDEADBEEF;
    size_t iterations = 1000;
    
    /* Phase 1: Sequential write */
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (int)(i & 0xFF);
    }
    COMPILER_BARRIER();
    
    /* Phase 2: Pseudo-random read/write pattern */
    for (size_t iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < size; i += 64) { /* 64-byte cache line stride */
            uint32_t idx = lcg(&seed) % size;
            buffer[idx] = buffer[(idx + 1) % size] + 1;
        }
    }
    COMPILER_BARRIER();
    
    /* Phase 3: Summation to force memory reads */
    int sum = 0;
    for (size_t i = 0; i < size; i += 8) { /* Strided access */
        sum += buffer[i];
    }
    
    g_result = sum;
}

/* Main benchmark driver */
int main(void) {
    const size_t l1_size = 32 * 1024;      /* Larger than typical L1 */
    const size_t l2_size = 512 * 1024;     /* Larger than typical L2 */
    const size_t buffer_size = l2_size * 4; /* 2MB to exceed L2 */
    
    int *buffer1 = NULL;
    int *buffer2 = NULL;
    clock_t start, end;
    double elapsed;
    
    /* Allocate large buffers to ensure cache pressure */
    buffer1 = (int*)malloc(buffer_size * sizeof(int));
    buffer2 = (int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize buffers with non-zero data */
    for (size_t i = 0; i < buffer_size; i++) {
        buffer1[i] = (int)(i & 0xFF);
        buffer2[i] = (int)((i + 1) & 0xFF);
    }
    
    printf("Starting cache benchmark...\n");
    
    /* Run benchmark multiple times to ensure driver optimization decisions */
    for (int run = 0; run < 3; run++) {
        start = clock();
        
        /* Interleave operations on both buffers */
        for (size_t block = 0; block < buffer_size; block += l1_size) {
            size_t chunk_size = (block + l1_size > buffer_size) ? 
                               (buffer_size - block) : l1_size;
            
            /* Process buffer1 */
            perform_cache_benchmark(buffer1 + block, chunk_size);
            
            /* Process buffer2 */
            perform_cache_benchmark(buffer2 + block, chunk_size);
            
            /* Cross-buffer operations to increase cache pressure */
            for (size_t i = 0; i < chunk_size; i += 16) {
                buffer2[block + i] = buffer1[block + i] + buffer2[block + i];
            }
        }
        
        end = clock();
        elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        printf("Run %d: %.3f seconds, result = %d\n", 
               run + 1, elapsed, g_result);
        
        /* Shuffle data between runs */
        for (size_t i = 0; i < buffer_size; i += 2) {
            int temp = buffer1[i];
            buffer1[i] = buffer2[i + 1];
            buffer2[i + 1] = temp;
        }
    }
    
    /* Final computation that uses all data */
    int final_sum = 0;
    for (size_t i = 0; i < buffer_size; i += 4) {
        final_sum += buffer1[i] * buffer2[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    
    free(buffer1);
    free(buffer2);
    
    return 0;
}

/* Additional target-specific variants for multi-versioning */
#if defined(ENABLE_MULTIVERSION) && __GNUC__ >= 4

/* Pentium III variant - targets cases 0x0a, 0x0c, 0x0d, 0x0e */
__attribute__((target("arch=pentium3")))
void benchmark_pentium3(int *buffer, size_t size) {
    perform_cache_benchmark(buffer, size);
}

/* Pentium 4 variant - targets cases 0x21, 0x24, 0x39-0x3e, 0x41-0x45 */
__attribute__((target("arch=pentium4")))
void benchmark_pentium4(int *buffer, size_t size) {
    perform_cache_benchmark(buffer, size);
}

/* Nocona (Xeon DP) variant - targets case 0x49 (non-Xeon-MP) */
__attribute__((target("arch=nocona")))
void benchmark_nocona(int *buffer, size_t size) {
    perform_cache_benchmark(buffer, size);
}

/* AMD K8 variant - targets cases 0x40 series, 0x78-0x87 */
__attribute__((target("arch=k8")))
void benchmark_k8(int *buffer, size_t size) {
    perform_cache_benchmark(buffer, size);
}

/* Core 2 variant - targets cases 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void benchmark_core2(int *buffer, size_t size) {
    perform_cache_benchmark(buffer, size);
}

#endif /* ENABLE_MULTIVERSION */
