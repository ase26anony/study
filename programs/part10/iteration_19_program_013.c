/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Target-specific function attributes */
#ifdef TEST_PENTIUM3
#define TARGET_ATTR __attribute__((target("arch=pentium3")))
#elif defined(TEST_PENTIUM4)
#define TARGET_ATTR __attribute__((target("arch=pentium4")))
#elif defined(TEST_NOCONA)
#define TARGET_ATTR __attribute__((target("arch=nocona")))
#elif defined(TEST_K8)
#define TARGET_ATTR __attribute__((target("arch=k8")))
#elif defined(TEST_CORE2)
#define TARGET_ATTR __attribute__((target("arch=core2")))
#elif defined(TEST_NEHALEM)
#define TARGET_ATTR __attribute__((target("arch=nehalem")))
#else
#define TARGET_ATTR
#endif

/* Cache thrashing benchmark function */
TARGET_ATTR
static void cache_thrash_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Pseudo-random access pattern using linear congruential generator */
    uint32_t seed = 0xDEADBEEF;
    
    for (j = 0; j < iterations; j++) {
        /* Write phase - fill buffer with pattern */
        for (i = 0; i < size; i++) {
            buffer[i] = (int)(seed ^ i);
            seed = seed * 1103515245 + 12345;
        }
        
        COMPILER_BARRIER();
        
        /* Read-modify-write phase with stride */
        for (i = 0; i < size; i += 64) {  /* 64-byte cache line stride */
            buffer[i] = buffer[i] * 3 + 1;
        }
        
        COMPILER_BARRIER();
        
        /* Accumulate results to prevent elimination */
        for (i = 0; i < size; i += 128) {
            sink += buffer[i];
        }
    }
    
    /* Use sink to prevent dead code elimination */
    if (sink == 0x12345678) {
        printf("Impossible\n");
    }
}

/* Specialized benchmarks for different cache configurations */
#ifdef TEST_CASE_0x0A
/* Targets: Intel Pentium III (Coppermine) - 8KB L1D */
__attribute__((target("arch=pentium3")))
static void bench_case_0x0a(void) {
    size_t buffer_size = 16 * 1024;  /* Exceeds L1 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 1000);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x0C
/* Targets: Intel Pentium III (Tualatin) - 16KB L1D */
__attribute__((target("arch=pentium3")))
static void bench_case_0x0c(void) {
    size_t buffer_size = 32 * 1024;  /* Exceeds L1 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 1000);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x21
/* Targets: Intel Pentium 4 (Willamette) - 256KB L2 */
__attribute__((target("arch=pentium4")))
static void bench_case_0x21(void) {
    size_t buffer_size = 512 * 1024;  /* Exceeds L2 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 500);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x24
/* Targets: Intel Pentium 4 (Northwood) - 1MB L2 */
__attribute__((target("arch=pentium4")))
static void bench_case_0x24(void) {
    size_t buffer_size = 2 * 1024 * 1024;  /* Exceeds L2 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 200);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x49
/* Targets: Intel Xeon DP (Nocona) - 4MB L2 (non-MP) */
__attribute__((target("arch=nocona")))
static void bench_case_0x49(void) {
    size_t buffer_size = 8 * 1024 * 1024;  /* Exceeds L2 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 100);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x60
/* Targets: AMD K8 (Athlon 64) - 16KB L1D */
__attribute__((target("arch=k8")))
static void bench_case_0x60(void) {
    size_t buffer_size = 32 * 1024;  /* Exceeds L1 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 1000);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x78
/* Targets: AMD K10 (Phenom) - 1MB L2 */
__attribute__((target("arch=amdfam10")))
static void bench_case_0x78(void) {
    size_t buffer_size = 2 * 1024 * 1024;  /* Exceeds L2 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 200);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x7A
/* Targets: Intel Core 2 (Merom) - 256KB L2 */
__attribute__((target("arch=core2")))
static void bench_case_0x7a(void) {
    size_t buffer_size = 512 * 1024;  /* Exceeds L2 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 500);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x86
/* Targets: Intel Nehalem - 512KB L2 */
__attribute__((target("arch=nehalem")))
static void bench_case_0x86(void) {
    size_t buffer_size = 1 * 1024 * 1024;  /* Exceeds L2 */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (buffer) {
        cache_thrash_benchmark(buffer, buffer_size, 300);
        free(buffer);
    }
}
#endif

/* Main benchmark driver */
TARGET_ATTR
int main(void) {
    volatile int result = 0;
    size_t i;
    
    /* Allocate buffers of different sizes to stress all cache levels */
    size_t sizes[] = {
        8 * 1024,      /* ~L1 size */
        64 * 1024,     /* Between L1-L2 */
        512 * 1024,    /* ~L2 size */
        4 * 1024 * 1024, /* > L2, possibly L3 */
        16 * 1024 * 1024 /* Definitely exceeds all caches */
    };
    
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    /* Run conditional benchmarks based on compile-time definitions */
#if defined(TEST_CASE_0x0A)
    bench_case_0x0a();
#endif
#if defined(TEST_CASE_0x0C)
    bench_case_0x0c();
#endif
#if defined(TEST_CASE_0x21)
    bench_case_0x21();
#endif
#if defined(TEST_CASE_0x24)
    bench_case_0x24();
#endif
#if defined(TEST_CASE_0x49)
    bench_case_0x49();
#endif
#if defined(TEST_CASE_0x60)
    bench_case_0x60();
#endif
#if defined(TEST_CASE_0x78)
    bench_case_0x78();
#endif
#if defined(TEST_CASE_0x7A)
    bench_case_0x7a();
#endif
#if defined(TEST_CASE_0x86)
    bench_case_0x86();
#endif
    
    /* Generic cache thrashing test */
    for (i = 0; i < num_sizes; i++) {
        int *buffer = malloc(sizes[i] * sizeof(int));
        if (!buffer) continue;
        
        /* Initialize with pattern */
        for (size_t j = 0; j < sizes[i]; j++) {
            buffer[j] = (int)(j ^ 0x55AA55AA);
        }
        
        COMPILER_BARRIER();
        
        /* Access with various strides to test associativity */
        const size_t strides[] = {1, 2, 4, 8, 16, 32, 64, 128};
        for (size_t s = 0; s < sizeof(strides)/sizeof(strides[0]); s++) {
            size_t stride = strides[s];
            volatile int sum = 0;
            
            for (size_t j = 0; j < sizes[i]; j += stride) {
                sum += buffer[j];
                buffer[j] = sum;
            }
            
            result += sum;
            COMPILER_BARRIER();
        }
        
        free(buffer);
    }
    
    /* Print result to prevent elimination */
    printf("Cache benchmark result: %d\n", result);
    
    return 0;
}
