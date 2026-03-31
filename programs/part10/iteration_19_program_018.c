/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our benchmarks */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
/* Targets: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
/* Targets: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68 */
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
/* Targets: 0x78-0x87 */
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
/* Targets: 0x48, 0x4e */
__attribute__((target("arch=core2")))
#endif
static void benchmark_cache(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Simple cache-thrashing pattern */
    for (j = 0; j < iterations; j++) {
        /* Sequential access pattern */
        for (i = 0; i < size; i += 64) {
            buffer[i] = buffer[i] * 3 + 1;
        }
        COMPILER_BARRIER();
        
        /* Strided access pattern (tests associativity) */
        for (i = 0; i < size; i += 128) {
            buffer[i] = buffer[i] * 2 - 1;
        }
        COMPILER_BARRIER();
        
        /* Pseudo-random access pattern */
        unsigned int seed = j;
        for (i = 0; i < size / 4; i++) {
            seed = seed * 1103515245 + 12345;
            size_t idx = (seed ^ (seed >> 16)) % size;
            buffer[idx] = buffer[idx] + seed;
        }
        COMPILER_BARRIER();
    }
    
    /* Prevent dead code elimination */
    for (i = 0; i < size; i++) {
        sink ^= buffer[i];
    }
}

/* Multi-versioned function for different CPU targets */
#if defined(USE_MULTIVERSIONING) && __GNUC__ >= 6
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void multiarch_benchmark() {
    const size_t l1_size = 32 * 1024;      /* Larger than typical L1 */
    const size_t l2_size = 512 * 1024;     /* Larger than typical L2 */
    const int iterations = 100;
    
    int *buffer1 = (int*)malloc(l1_size * sizeof(int));
    int *buffer2 = (int*)malloc(l2_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with non-zero values */
    for (size_t i = 0; i < l1_size; i++) {
        buffer1[i] = (i * 3) % 256;
    }
    for (size_t i = 0; i < l2_size; i++) {
        buffer2[i] = (i * 5) % 256;
    }
    
    /* Run benchmarks that should trigger cache detection */
    benchmark_cache(buffer1, l1_size, iterations);
    benchmark_cache(buffer2, l2_size, iterations / 10);
    
    /* Mix both buffers */
    for (int i = 0; i < iterations / 20; i++) {
        for (size_t j = 0; j < l1_size; j++) {
            buffer2[j % l2_size] += buffer1[j];
        }
        COMPILER_BARRIER();
    }
    
    free(buffer1);
    free(buffer2);
}

/* Individual test functions for specific cache descriptor cases */
#ifdef TEST_0x0A
__attribute__((target("arch=pentium3")))
void test_case_0x0a() {
    /* Should trigger case 0x0a: L1 8KB, 2-way, 32B line */
    volatile int array[2048]; /* 8KB */
    for (int i = 0; i < 2048; i++) array[i] = i;
    COMPILER_BARRIER();
}
#endif

#ifdef TEST_0x49
__attribute__((target("arch=nocona")))
void test_case_0x49() {
    /* Should trigger case 0x49 (non-Xeon-MP): L2 4MB, 16-way, 64B line */
    volatile int *large = malloc(4 * 1024 * 1024);
    if (large) {
        for (int i = 0; i < 1024 * 1024; i++) large[i] = i;
        free((void*)large);
    }
    COMPILER_BARRIER();
}
#endif

#ifdef TEST_0x78_0x87
__attribute__((target("arch=k8")))
void test_case_0x78_0x87() {
    /* Should trigger AMD K8 cache cases: 0x78-0x87 */
    volatile int *buf = malloc(2 * 1024 * 1024);
    if (buf) {
        /* Access pattern that tests L2 cache */
        for (int stride = 64; stride <= 4096; stride *= 2) {
            for (int i = 0; i < 2 * 1024 * 1024; i += stride) {
                buf[i] = buf[i] * 2;
            }
        }
        free((void*)buf);
    }
    COMPILER_BARRIER();
}
#endif

int main(int argc, char **argv) {
    clock_t start, end;
    
    printf("Cache detection coverage test\n");
    
    /* Run multi-arch benchmark if enabled */
#ifdef USE_MULTIVERSIONING
    printf("Running multi-architecture benchmarks...\n");
    start = clock();
    multiarch_benchmark();
    end = clock();
    printf("Multi-arch benchmark time: %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
#endif
    
    /* Run specific test cases */
#ifdef TEST_0x0A
    printf("Testing case 0x0a (Pentium III L1)...\n");
    test_case_0x0a();
#endif
    
#ifdef TEST_0x49
    printf("Testing case 0x49 (Nocona L2)...\n");
    test_case_0x49();
#endif
    
#ifdef TEST_0x78_0x87
    printf("Testing cases 0x78-0x87 (AMD K8)...\n");
    test_case_0x78_0x87();
#endif
    
    /* Generic cache stress test */
    printf("Running generic cache stress test...\n");
    const size_t test_size = 1024 * 1024; /* 1MB */
    int *test_buf = (int*)malloc(test_size * sizeof(int));
    
    if (test_buf) {
        start = clock();
        
        /* Fill with values */
        for (size_t i = 0; i < test_size; i++) {
            test_buf[i] = (i * 7) % 256;
        }
        
        /* Cache-thrashing access pattern */
        volatile int sum = 0;
        for (int iter = 0; iter < 50; iter++) {
            /* Sequential */
            for (size_t i = 0; i < test_size; i++) {
                test_buf[i] = test_buf[i] * 3 + 1;
            }
            COMPILER_BARRIER();
            
            /* Reverse */
            for (size_t i = test_size - 1; i > 0; i--) {
                test_buf[i] = test_buf[i] * 2 - 1;
            }
            COMPILER_BARRIER();
            
            /* Strided */
            for (size_t i = 0; i < test_size; i += 67) {
                sum += test_buf[i];
            }
        }
        
        end = clock();
        printf("Generic test time: %f seconds (sum=%d)\n", 
               (double)(end - start) / CLOCKS_PER_SEC, sum);
        
        free(test_buf);
    }
    
    return 0;
}
