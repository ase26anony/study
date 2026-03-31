/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPUID leaf 2 cache detection
 * logic by targeting specific x86 microarchitectures through compiler flags.
 * Each architecture section corresponds to specific cache descriptor bytes
 * that should trigger the uncovered case statements in driver-i386.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away memory accesses */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function template */
static void cache_thrash_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Use a simple LCG for pseudo-random access pattern */
    uint32_t state = 0xDEADBEEF;
    
    for (j = 0; j < iterations; j++) {
        /* Linear access (good for prefetch) */
        for (i = 0; i < size; i++) {
            buffer[i] = i;
        }
        MEMORY_BARRIER();
        
        /* Pseudo-random access (stress associativity) */
        for (i = 0; i < size; i++) {
            /* Simple LCG: state = state * 1103515245 + 12345 */
            state = state * 1103515245U + 12345U;
            size_t idx = (state >> 16) % size;
            sink += buffer[idx];
            buffer[idx] = sink;
        }
        MEMORY_BARRIER();
        
        /* Strided access (check line size) */
        for (i = 0; i < size; i += 64 / sizeof(int)) {
            sink += buffer[i];
        }
        MEMORY_BARRIER();
    }
    
    /* Use sink to prevent dead code elimination */
    if (sink == 0x12345678) {
        printf("Impossible condition\n");
    }
}

/* ============================================================================
 * Architecture-specific implementations using target attributes
 * ============================================================================ */

/* Case 0x0a: 8KB L1, 2-way, 32B line (Pentium III, some Celeron) */
#ifdef TEST_CASE_0x0A
__attribute__((target("arch=pentium3")))
static void bench_case_0x0a(void) {
    printf("Testing cache config for descriptor 0x0a (8KB L1, 2-way, 32B line)\n");
    size_t size = 2 * 1024 * 1024 / sizeof(int); /* 2MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 100);
        free(buffer);
    }
}
#endif

/* Case 0x0c: 16KB L1, 4-way, 32B line (Pentium III, some Pentium 4) */
#ifdef TEST_CASE_0x0C
__attribute__((target("arch=pentium3")))
static void bench_case_0x0c(void) {
    printf("Testing cache config for descriptor 0x0c (16KB L1, 4-way, 32B line)\n");
    size_t size = 4 * 1024 * 1024 / sizeof(int); /* 4MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 100);
        free(buffer);
    }
}
#endif

/* Case 0x21: 256KB L2, 8-way, 64B line (Pentium 4, Xeon) */
#ifdef TEST_CASE_0x21
__attribute__((target("arch=pentium4")))
static void bench_case_0x21(void) {
    printf("Testing cache config for descriptor 0x21 (256KB L2, 8-way, 64B line)\n");
    size_t size = 8 * 1024 * 1024 / sizeof(int); /* 8MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 50);
        free(buffer);
    }
}
#endif

/* Case 0x24: 1MB L2, 16-way, 64B line (Pentium 4, Xeon) */
#ifdef TEST_CASE_0x24
__attribute__((target("arch=pentium4")))
static void bench_case_0x24(void) {
    printf("Testing cache config for descriptor 0x24 (1MB L2, 16-way, 64B line)\n");
    size_t size = 16 * 1024 * 1024 / sizeof(int); /* 16MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 30);
        free(buffer);
    }
}
#endif

/* Case 0x2c: 32KB L1, 8-way, 64B line (Pentium 4 Prescott) */
#ifdef TEST_CASE_0x2C
__attribute__((target("arch=prescott")))
static void bench_case_0x2c(void) {
    printf("Testing cache config for descriptor 0x2c (32KB L1, 8-way, 64B line)\n");
    size_t size = 8 * 1024 * 1024 / sizeof(int); /* 8MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 50);
        free(buffer);
    }
}
#endif

/* Case 0x39: 128KB L2, 4-way, 64B line (Pentium M, some Celeron) */
#ifdef TEST_CASE_0x39
__attribute__((target("arch=pentium-m")))
static void bench_case_0x39(void) {
    printf("Testing cache config for descriptor 0x39 (128KB L2, 4-way, 64B line)\n");
    size_t size = 4 * 1024 * 1024 / sizeof(int); /* 4MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 80);
        free(buffer);
    }
}
#endif

/* Case 0x49: 4MB L2, 16-way, 64B line (Xeon DP, not MP) */
#ifdef TEST_CASE_0x49
/* Use nocona (Xeon DP) which should not set xeon_mp flag */
__attribute__((target("arch=nocona")))
static void bench_case_0x49(void) {
    printf("Testing cache config for descriptor 0x49 (4MB L2, 16-way, 64B line)\n");
    size_t size = 32 * 1024 * 1024 / sizeof(int); /* 32MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 20);
        free(buffer);
    }
}
#endif

/* Case 0x60: 16KB L1, 8-way, 64B line (AMD K8) */
#ifdef TEST_CASE_0x60
__attribute__((target("arch=k8")))
static void bench_case_0x60(void) {
    printf("Testing cache config for descriptor 0x60 (16KB L1, 8-way, 64B line)\n");
    size_t size = 8 * 1024 * 1024 / sizeof(int); /* 8MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 60);
        free(buffer);
    }
}
#endif

/* Case 0x78: 1MB L2, 4-way, 64B line (AMD K8, some Intel) */
#ifdef TEST_CASE_0x78
__attribute__((target("arch=k8")))
static void bench_case_0x78(void) {
    printf("Testing cache config for descriptor 0x78 (1MB L2, 4-way, 64B line)\n");
    size_t size = 16 * 1024 * 1024 / sizeof(int); /* 16MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 30);
        free(buffer);
    }
}
#endif

/* Case 0x86: 512KB L2, 4-way, 64B line (Core 2 Duo) */
#ifdef TEST_CASE_0x86
__attribute__((target("arch=core2")))
static void bench_case_0x86(void) {
    printf("Testing cache config for descriptor 0x86 (512KB L2, 4-way, 64B line)\n");
    size_t size = 8 * 1024 * 1024 / sizeof(int); /* 8MB buffer */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 40);
        free(buffer);
    }
}
#endif

/* ============================================================================
 * Main function with conditional compilation
 * ============================================================================ */

int main(void) {
    printf("Cache Detection Test Program\n");
    printf("============================\n\n");
    
    /* Generic benchmark that should work on any architecture */
    printf("Running generic cache benchmark...\n");
    size_t generic_size = 16 * 1024 * 1024 / sizeof(int); /* 16MB */
    int *generic_buffer = malloc(generic_size * sizeof(int));
    if (generic_buffer) {
        cache_thrash_benchmark(generic_buffer, generic_size, 10);
        free(generic_buffer);
    }
    
    /* Call architecture-specific benchmarks based on compile-time defines */
#ifdef TEST_CASE_0x0A
    bench_case_0x0a();
#endif
    
#ifdef TEST_CASE_0x0C
    bench_case_0x0c();
#endif
    
#ifdef TEST_CASE_0x21
    bench_case_0x21();
#endif
    
#ifdef TEST_CASE_0x24
    bench_case_0x24();
#endif
    
#ifdef TEST_CASE_0x2C
    bench_case_0x2c();
#endif
    
#ifdef TEST_CASE_0x39
    bench_case_0x39();
#endif
    
#ifdef TEST_CASE_0x49
    bench_case_0x49();
#endif
    
#ifdef TEST_CASE_0x60
    bench_case_0x60();
#endif
    
#ifdef TEST_CASE_0x78
    bench_case_0x78();
#endif
    
#ifdef TEST_CASE_0x86
    bench_case_0x86();
#endif
    
    printf("\nBenchmark completed.\n");
    return 0;
}
