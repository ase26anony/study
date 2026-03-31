/* test_cache_coverage.c
 * 
 * A comprehensive test to exercise GCC i386 driver cache detection logic
 * for specific CPUID leaf 2 descriptor values.
 * 
 * Compile with different -D flags and -march options to target various CPUs:
 * 
 * For case 0x0a (8KB L1):  gcc -DTEST_CASE_0x0A -march=pentium3 -O2 test_cache_coverage.c -o test_0x0a
 * For case 0x0c (16KB L1): gcc -DTEST_CASE_0x0C -march=pentium3 -O2 test_cache_coverage.c -o test_0x0c
 * For case 0x21 (256KB L2): gcc -DTEST_CASE_0x21 -march=pentium3 -O2 test_cache_coverage.c -o test_0x21
 * For case 0x49 (4MB L2):  gcc -DTEST_CASE_0x49 -march=nocona -O2 test_cache_coverage.c -o test_0x49
 * For case 0x78 (1MB L2):  gcc -DTEST_CASE_0x78 -march=k8 -O2 test_cache_coverage.c -o test_0x78
 * 
 * Or compile all together: gcc -DTEST_ALL -march=x86-64 -O2 test_cache_coverage.c -o test_all
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* Memory barrier to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache line size assumption for padding */
#define CACHE_LINE_SIZE 64

/* Different benchmark patterns to exercise various cache behaviors */
typedef enum {
    PATTERN_SEQUENTIAL,
    PATTERN_STRIDE,
    PATTERN_RANDOM,
    PATTERN_BLOCK
} access_pattern_t;

/* Benchmark configuration */
typedef struct {
    size_t buffer_size;      /* In elements */
    size_t iterations;
    access_pattern_t pattern;
    size_t stride;
    const char *cpu_name;
} benchmark_config_t;

/* Function attributes for targeting specific CPUs */
#ifdef __GNUC__

/* Pentium III target - should trigger cases 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
#ifdef TEST_CASE_0x0A
__attribute__((target("arch=pentium3")))
#endif
static void benchmark_pentium3(const benchmark_config_t *config) {
    volatile int result = 0;
    size_t i, j;
    
    /* Allocate buffer aligned to cache line */
    int *buffer = aligned_alloc(CACHE_LINE_SIZE, 
                               config->buffer_size * sizeof(int));
    if (!buffer) {
        fprintf(stderr, "Allocation failed for Pentium3 benchmark\n");
        return;
    }
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < config->buffer_size; i++) {
        buffer[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    COMPILER_BARRIER();
    
    /* Perform benchmark based on pattern */
    switch (config->pattern) {
        case PATTERN_SEQUENTIAL:
            for (j = 0; j < config->iterations; j++) {
                for (i = 0; i < config->buffer_size; i++) {
                    result += buffer[i];
                    buffer[i] = result;
                }
                COMPILER_BARRIER();
            }
            break;
            
        case PATTERN_STRIDE:
            for (j = 0; j < config->iterations; j++) {
                for (i = 0; i < config->buffer_size; i += config->stride) {
                    result += buffer[i];
                    buffer[i] = result;
                }
                COMPILER_BARRIER();
            }
            break;
            
        default:
            /* Linear access for Pentium3 */
            for (j = 0; j < config->iterations; j++) {
                for (i = 0; i < config->buffer_size; i++) {
                    result += buffer[i];
                    /* Simple transformation to prevent dead code elimination */
                    buffer[i] = (buffer[i] * 13 + 7) & 0xFF;
                }
                COMPILER_BARRIER();
            }
            break;
    }
    
    /* Use result to prevent optimization */
    printf("Pentium3 benchmark result: %d\n", result);
    
    free(buffer);
}

/* Nocona/Xeon DP target - should trigger case 0x49 (4MB L2, non-Xeon-MP) */
#ifdef TEST_CASE_0x49
__attribute__((target("arch=nocona")))
#endif
static void benchmark_nocona(const benchmark_config_t *config) {
    volatile int result = 0;
    size_t i, j, k;
    
    /* Larger buffers for Xeon-class CPUs */
    int *buffer1 = aligned_alloc(CACHE_LINE_SIZE, 
                                config->buffer_size * sizeof(int));
    int *buffer2 = aligned_alloc(CACHE_LINE_SIZE, 
                                config->buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Allocation failed for Nocona benchmark\n");
        free(buffer1);
        free(buffer2);
        return;
    }
    
    /* Initialize buffers */
    for (i = 0; i < config->buffer_size; i++) {
        buffer1[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        buffer2[i] = (i * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    COMPILER_BARRIER();
    
    /* Matrix-style access pattern to exercise larger caches */
    for (j = 0; j < config->iterations; j++) {
        /* Transpose-like operation */
        for (i = 0; i < config->buffer_size; i += 64) {
            for (k = 0; k < 64 && (i + k) < config->buffer_size; k++) {
                int temp = buffer1[i + k];
                buffer1[i + k] = buffer2[(i + k) % config->buffer_size];
                buffer2[(i + k) % config->buffer_size] = temp;
                result += temp;
            }
        }
        COMPILER_BARRIER();
    }
    
    printf("Nocona benchmark result: %d\n", result);
    
    free(buffer1);
    free(buffer2);
}

/* AMD K8 target - should trigger cases 0x78-0x87 */
#ifdef TEST_CASE_0x78
__attribute__((target("arch=k8")))
#endif
static void benchmark_k8(const benchmark_config_t *config) {
    volatile int result = 0;
    size_t i, j;
    
    /* Use multiple smaller buffers for AMD's cache hierarchy */
    const int num_buffers = 8;
    int **buffers = malloc(num_buffers * sizeof(int *));
    
    if (!buffers) {
        fprintf(stderr, "Allocation failed for K8 benchmark\n");
        return;
    }
    
    for (int b = 0; b < num_buffers; b++) {
        buffers[b] = aligned_alloc(CACHE_LINE_SIZE, 
                                  (config->buffer_size / num_buffers) * sizeof(int));
        if (!buffers[b]) {
            fprintf(stderr, "Buffer allocation failed\n");
            for (int k = 0; k < b; k++) free(buffers[k]);
            free(buffers);
            return;
        }
        
        /* Initialize each buffer differently */
        for (i = 0; i < config->buffer_size / num_buffers; i++) {
            buffers[b][i] = ((b * 1000 + i) * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    COMPILER_BARRIER();
    
    /* Interleave access across multiple buffers */
    for (j = 0; j < config->iterations; j++) {
        for (int b = 0; b < num_buffers; b++) {
            size_t buffer_size = config->buffer_size / num_buffers;
            for (i = 0; i < buffer_size; i += config->stride) {
                result += buffers[b][i];
                /* XOR pattern to create dependencies */
                buffers[b][i] ^= result;
                buffers[(b + 1) % num_buffers][i % buffer_size] += result;
            }
        }
        COMPILER_BARRIER();
    }
    
    printf("K8 benchmark result: %d\n", result);
    
    for (int b = 0; b < num_buffers; b++) {
        free(buffers[b]);
    }
    free(buffers);
}

/* Generic benchmark that uses multi-versioning */
__attribute__((target_clones("default,arch=pentium3,arch=nocona,arch=k8")))
static void benchmark_multiarch(const benchmark_config_t *config) {
    volatile int result = 0;
    size_t i, j;
    
    int *buffer = aligned_alloc(CACHE_LINE_SIZE, 
                               config->buffer_size * sizeof(int));
    if (!buffer) {
        fprintf(stderr, "Allocation failed for multiarch benchmark\n");
        return;
    }
    
    /* Simple linear access that will be optimized differently per arch */
    for (i = 0; i < config->buffer_size; i++) {
        buffer[i] = i;
    }
    
    COMPILER_BARRIER();
    
    /* Access pattern that benefits from cache awareness */
    for (j = 0; j < config->iterations; j++) {
        /* Blocked access pattern */
        const size_t block_size = 256; /* Should be tuned per cache */
        for (size_t block_start = 0; block_start < config->buffer_size; block_start += block_size) {
            size_t block_end = block_start + block_size;
            if (block_end > config->buffer_size) block_end = config->buffer_size;
            
            for (i = block_start; i < block_end; i++) {
                result += buffer[i];
                buffer[i] = (buffer[i] * 3 + 1) & 0xFFF;
            }
        }
        COMPILER_BARRIER();
    }
    
    printf("Multiarch benchmark result: %d\n", result);
    free(buffer);
}

#endif /* __GNUC__ */

/* Main test driver */
int main(int argc, char *argv[]) {
    benchmark_config_t config;
    
    /* Configure based on compile-time definitions */
#ifdef TEST_CASE_0x0A
    /* Target small L1 cache (8KB) */
    config.buffer_size = 8192 * 2; /* 2x L1 to ensure evictions */
    config.iterations = 10000;
    config.pattern = PATTERN_SEQUENTIAL;
    config.stride = 1;
    config.cpu_name = "Pentium3 (case 0x0a)";
    
    printf("Running benchmark for %s\n", config.cpu_name);
    benchmark_pentium3(&config);
    
#elif defined(TEST_CASE_0x0C)
    /* Target 16KB L1 cache */
    config.buffer_size = 16384 * 2;
    config.iterations = 8000;
    config.pattern = PATTERN_STRIDE;
    config.stride = 4;
    config.cpu_name = "Pentium3 (case 0x0c)";
    
    printf("Running benchmark for %s\n", config.cpu_name);
    benchmark_pentium3(&config);
    
#elif defined(TEST_CASE_0x21)
    /* Target 256KB L2 cache */
    config.buffer_size = 262144 * 2; /* 2x L2 */
    config.iterations = 2000;
    config.pattern = PATTERN_BLOCK;
    config.stride = 16;
    config.cpu_name = "Pentium3 (case 0x21)";
    
    printf("Running benchmark for %s\n", config.cpu_name);
    benchmark_pentium3(&config);
    
#elif defined(TEST_CASE_0x49)
    /* Target 4MB L2 cache (non-Xeon-MP) */
    config.buffer_size = 4194304 * 2; /* 2x 4MB */
    config.iterations = 500;
    config.pattern = PATTERN_RANDOM;
    config.stride = 67; /* Prime stride */
    config.cpu_name = "Nocona/Xeon DP (case 0x49)";
    
    printf("Running benchmark for %s\n", config.cpu_name);
    benchmark_nocona(&config);
    
#elif defined(TEST_CASE_0x78)
    /* Target 1MB L2 cache (AMD K8) */
    config.buffer_size = 1048576 * 2; /* 2x 1MB */
    config.iterations = 1000;
    config.pattern = PATTERN_STRIDE;
    config.stride = 13; /* Prime stride */
    config.cpu_name = "AMD K8 (case 0x78)";
    
    printf("Running benchmark for %s\n", config.cpu_name);
    benchmark_k8(&config);
    
#elif defined(TEST_ALL)
    /* Run all benchmarks sequentially */
    printf("Running comprehensive cache benchmark suite...\n");
    
    /* Test small cache configurations */
    config.buffer_size = 8192 * 2;
    config.iterations = 5000;
    config.pattern = PATTERN_SEQUENTIAL;
    config.stride = 1;
    
    printf("\n1. Testing Pentium3 target (cases 0x0a, 0x0c, etc.)\n");
    benchmark_pentium3(&config);
    
    /* Test medium cache */
    config.buffer_size = 262144 * 2;
    config.iterations = 1000;
    config.pattern = PATTERN_STRIDE;
    config.stride = 8;
    
    printf("\n2. Testing medium cache (cases 0x21, 0x24, etc.)\n");
    benchmark_pentium3(&config);
    
    /* Test large cache (Xeon) */
    config.buffer_size = 4194304 * 2;
    config.iterations = 200;
    config.pattern = PATTERN_BLOCK;
    config.stride = 64;
    
    printf("\n3. Testing Xeon DP (case 0x49)\n");
    benchmark_nocona(&config);
    
    /* Test AMD cache */
    config.buffer_size = 1048576 * 2;
    config.iterations = 500;
    config.pattern = PATTERN_STRIDE;
    config.stride = 17;
    
    printf("\n4. Testing AMD K8 (cases 0x78-0x87)\n");
    benchmark_k8(&config);
    
    /* Test multi-versioning */
    config.buffer_size = 524288; /* 512KB */
    config.iterations = 1000;
    config.pattern = PATTERN_SEQUENTIAL;
    config.stride = 1;
    
    printf("\n5. Testing multi-arch function\n");
    benchmark_multiarch(&config);
    
#else
    /* Default: generic benchmark */
    config.buffer_size = 1048576; /* 1MB */
    config.iterations = 1000;
    config.pattern = PATTERN_SEQUENTIAL;
    config.stride = 1;
    config.cpu_name = "Generic x86";
    
    printf("Running generic cache benchmark\n");
    benchmark_multiarch(&config);
#endif
    
    return 0;
}
