/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets for multi-versioning */
#ifdef TEST_PENTIUM3
#define CPU_TARGET __attribute__((target("arch=pentium3")))
#define CPU_NAME "Pentium III"
#elif defined(TEST_PENTIUM4)
#define CPU_TARGET __attribute__((target("arch=pentium4")))
#define CPU_NAME "Pentium 4"
#elif defined(TEST_NOCONA)
#define CPU_TARGET __attribute__((target("arch=nocona")))
#define CPU_NAME "Nocona (Xeon DP)"
#elif defined(TEST_K8)
#define CPU_TARGET __attribute__((target("arch=k8")))
#define CPU_NAME "AMD K8"
#elif defined(TEST_CORE2)
#define CPU_TARGET __attribute__((target("arch=core2")))
#define CPU_NAME "Core 2"
#elif defined(TEST_NEHALEM)
#define CPU_TARGET __attribute__((target("arch=nehalem")))
#define CPU_NAME "Nehalem"
#else
#define CPU_TARGET
#define CPU_NAME "Generic"
#endif

/* Cache-thrashing benchmark function with target-specific optimization */
CPU_TARGET static void cache_thrash_benchmark(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int size = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(size * sizeof(int));
    
    if (!buffer) return;
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (int i = 0; i < size; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    MB();
    
    /* Main benchmark: access pattern designed to stress cache */
    for (int iter = 0; iter < iterations; iter++) {
        /* Strided access pattern */
        for (int stride = 1; stride < 64; stride *= 2) {
            for (int i = 0; i < size; i += stride) {
                buffer[i] = buffer[i] * 3 + 1;
            }
            MB();
        }
        
        /* Reverse access pattern */
        for (int i = size - 1; i >= 0; i -= 17) {
            buffer[i] = buffer[i] / 2;
        }
        MB();
        
        /* Random-ish access using linear congruential generator */
        seed = iter;
        for (int i = 0; i < 10000; i++) {
            seed = seed * 1664525 + 1013904223;
            int idx = seed % size;
            buffer[idx] ^= 0x55555555;
        }
        MB();
    }
    
    /* Prevent dead code elimination */
    for (int i = 0; i < size; i += 128) {
        result ^= buffer[i];
    }
    
    /* Use result to prevent optimization */
    __asm__ __volatile__("" : "+r" (result) : : "memory");
    
    free(buffer);
}

/* Specialized benchmarks for different cache configurations */
#ifdef TEST_CASE_0x0A
/* Targets: Pentium III (Coppermine) - 8KB L1, 2-way */
CPU_TARGET static void bench_case_0x0a(void) {
    printf("Testing cache configuration 0x0A (8KB L1, 2-way, 32B line)\n");
    cache_thrash_benchmark(100, 8);  /* L1 size */
    cache_thrash_benchmark(50, 256); /* L2 size */
}
#endif

#ifdef TEST_CASE_0x0C
/* Targets: Pentium III - 16KB L1, 4-way */
CPU_TARGET static void bench_case_0x0c(void) {
    printf("Testing cache configuration 0x0C (16KB L1, 4-way, 32B line)\n");
    cache_thrash_benchmark(100, 16);
    cache_thrash_benchmark(50, 256);
}
#endif

#ifdef TEST_CASE_0x21
/* Targets: Pentium 4 - 256KB L2, 8-way */
CPU_TARGET static void bench_case_0x21(void) {
    printf("Testing cache configuration 0x21 (256KB L2, 8-way, 64B line)\n");
    cache_thrash_benchmark(50, 256);
    cache_thrash_benchmark(20, 1024);
}
#endif

#ifdef TEST_CASE_0x24
/* Targets: Pentium 4 - 1MB L2, 16-way */
CPU_TARGET static void bench_case_0x24(void) {
    printf("Testing cache configuration 0x24 (1MB L2, 16-way, 64B line)\n");
    cache_thrash_benchmark(30, 1024);
    cache_thrash_benchmark(10, 4096);
}
#endif

#ifdef TEST_CASE_0x49
/* Targets: Xeon DP (not MP) - 4MB L2, 16-way */
CPU_TARGET static void bench_case_0x49(void) {
    printf("Testing cache configuration 0x49 (4MB L2, 16-way, 64B line)\n");
    printf("Note: Should trigger unless xeon_mp flag is set\n");
    cache_thrash_benchmark(20, 4096);
    cache_thrash_benchmark(5, 16384);
}
#endif

#ifdef TEST_CASE_0x60
/* Targets: Core 2 - 16KB L1, 8-way */
CPU_TARGET static void bench_case_0x60(void) {
    printf("Testing cache configuration 0x60 (16KB L1, 8-way, 64B line)\n");
    cache_thrash_benchmark(100, 16);
    cache_thrash_benchmark(30, 2048);
}
#endif

#ifdef TEST_CASE_0x78
/* Targets: AMD K8 - 1MB L2, 4-way */
CPU_TARGET static void bench_case_0x78(void) {
    printf("Testing cache configuration 0x78 (1MB L2, 4-way, 64B line)\n");
    cache_thrash_benchmark(30, 1024);
    cache_thrash_benchmark(10, 4096);
}
#endif

#ifdef TEST_CASE_0x87
/* Targets: Various - 1MB L2, 8-way */
CPU_TARGET static void bench_case_0x87(void) {
    printf("Testing cache configuration 0x87 (1MB L2, 8-way, 64B line)\n");
    cache_thrash_benchmark(30, 1024);
    cache_thrash_benchmark(10, 4096);
}
#endif

/* Multi-version function using GCC's target clones */
#ifdef USE_MULTI_VERSIONING
__attribute__((target_clones("default,arch=pentium3,arch=pentium4,arch=nocona,arch=k8,arch=core2")))
static void multi_version_bench(void) {
    cache_thrash_benchmark(50, 1024);
}
#endif

int main(void) {
    printf("Cache detection coverage test for %s\n", CPU_NAME);
    printf("========================================\n");
    
    /* Force compiler to consider cache optimizations */
    volatile int dummy = 0;
    
    /* Run architecture-specific benchmarks */
#if defined(TEST_CASE_0x0A)
    bench_case_0x0a();
#elif defined(TEST_CASE_0x0C)
    bench_case_0x0c();
#elif defined(TEST_CASE_0x21)
    bench_case_0x21();
#elif defined(TEST_CASE_0x24)
    bench_case_0x24();
#elif defined(TEST_CASE_0x49)
    bench_case_0x49();
#elif defined(TEST_CASE_0x60)
    bench_case_0x60();
#elif defined(TEST_CASE_0x78)
    bench_case_0x78();
#elif defined(TEST_CASE_0x87)
    bench_case_0x87();
#else
    /* Generic benchmark that should work for all targets */
    printf("Running generic cache benchmark...\n");
    
    /* Test different buffer sizes to trigger various cache considerations */
    int sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        printf("  Testing with %dKB buffer...\n", sizes[i]);
        cache_thrash_benchmark(10, sizes[i]);
        MB();
    }
#endif
    
#ifdef USE_MULTI_VERSIONING
    printf("\nRunning multi-version benchmark...\n");
    multi_version_bench();
#endif
    
    /* Final memory operation to ensure all optimizations are considered */
    volatile int* final_check = (volatile int*)malloc(4096 * sizeof(int));
    if (final_check) {
        for (int i = 0; i < 4096; i++) {
            final_check[i] = i;
            dummy ^= final_check[i];
        }
        free((void*)final_check);
    }
    
    printf("\nBenchmark completed. Dummy result: %d\n", dummy);
    return 0;
}
