/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink;

/* Early CPU initialization - forces driver cache detection */
__attribute__((constructor))
static void init_cpu(void) {
    /* Initialize CPU detection - triggers driver's CPUID logic */
    __builtin_cpu_init();
    
    /* Query a wide range of CPU features to trigger various cache paths */
    cpu_features[0] = __builtin_cpu_supports("cmov");
    cpu_features[1] = __builtin_cpu_supports("mmx");
    cpu_features[2] = __builtin_cpu_supports("sse");
    cpu_features[3] = __builtin_cpu_supports("sse2");
    cpu_features[4] = __builtin_cpu_supports("sse3");
    cpu_features[5] = __builtin_cpu_supports("ssse3");
    cpu_features[6] = __builtin_cpu_supports("sse4.1");
    cpu_features[7] = __builtin_cpu_supports("sse4.2");
    cpu_features[8] = __builtin_cpu_supports("avx");
    cpu_features[9] = __builtin_cpu_supports("avx2");
    cpu_features[10] = __builtin_cpu_supports("avx512f");
    cpu_features[11] = __builtin_cpu_supports("fma");
    cpu_features[12] = __builtin_cpu_supports("aes");
    cpu_features[13] = __builtin_cpu_supports("pclmul");
    cpu_features[14] = __builtin_cpu_supports("rdrand");
    cpu_features[15] = __builtin_cpu_supports("rdseed");
    cpu_features[16] = __builtin_cpu_supports("sha");
    cpu_features[17] = __builtin_cpu_supports("xsave");
    cpu_features[18] = __builtin_cpu_supports("xsaveopt");
    cpu_features[19] = __builtin_cpu_supports("xsavec");
    cpu_features[20] = __builtin_cpu_supports("xsaves");
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void test_cache_core2(void) {
    volatile char buffer[32768];  /* 32KB - typical L1 size */
    for (int i = 0; i < sizeof(buffer); i += 64) {  /* 64-byte stride */
        sink = buffer[i];
    }
}

__attribute__((target("arch=sandybridge")))
static void test_cache_sandybridge(void) {
    volatile char buffer[262144];  /* 256KB - typical L2 size */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=haswell")))
static void test_cache_haswell(void) {
    volatile char buffer[1048576];  /* 1MB - typical L3 slice */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=skylake")))
static void test_cache_skylake(void) {
    volatile char buffer[2097152];  /* 2MB */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

/* Cache-sensitive memory access pattern */
static unsigned long long measure_cache_access(void) {
    const size_t size = 4 * 1024 * 1024;  /* 4MB - larger than typical L2 */
    char *buffer = malloc(size);
    unsigned long long start, end;
    
    if (!buffer) return 0;
    
    memset(buffer, 0, size);
    
    /* Time sequential access */
    start = __builtin_ia32_rdtsc();
    for (size_t i = 0; i < size; i += 64) {  /* 64-byte cache lines */
        sink = buffer[i];
    }
    end = __builtin_ia32_rdtsc();
    
    free(buffer);
    return end - start;
}

/* Test with different cache line assumptions */
static void test_cache_lines(void) {
    volatile char buffer[65536];
    
    /* Test 32-byte stride (for older CPUs) */
    for (int i = 0; i < sizeof(buffer); i += 32) {
        sink = buffer[i];
    }
    
    /* Test 64-byte stride (modern CPUs) */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
    
    /* Test 128-byte stride (prefetch) */
    for (int i = 0; i < sizeof(buffer); i += 128) {
        sink = buffer[i];
    }
}

/* Vectorized loop with pragma to trigger AVX detection */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    volatile int buffer[16384];
    for (int i = 0; i < 16384; i += 8) {  /* Process 8 ints at a time */
        sink = buffer[i];
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    volatile int buffer[16384];
    for (int i = 0; i < 16384; i += 16) {  /* Process 16 ints at a time */
        sink = buffer[i];
    }
}
#pragma GCC pop_options

int main(void) {
    unsigned long long checksum = 0;
    
    /* Call all target-specific functions */
    test_cache_core2();
    test_cache_sandybridge();
    test_cache_haswell();
    test_cache_skylake();
    
    /* Test different cache access patterns */
    test_cache_lines();
    
    /* Call vectorized functions */
    avx2_cache_test();
    avx512_cache_test();
    
    /* Measure cache performance - timing varies with actual cache size */
    unsigned long long cache_time = measure_cache_access();
    
    /* Build checksum from CPU features and timing */
    for (int i = 0; i < 21; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    for (int i = 0; i < 8; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    checksum ^= cache_time;
    
    /* Use checksum to prevent dead code elimination */
    if (checksum == 0) {
        printf("Zero checksum - unlikely\n");
    }
    
    printf("CPU detection test completed. Checksum: %llx\n", checksum);
    
    /* Additional feature queries to ensure driver paths are taken */
    if (__builtin_cpu_supports("sse4.2")) {
        /* This may trigger cache detection for SSE4.2 capable CPUs */
        volatile int x = 0;
        for (int i = 0; i < 1000; i++) {
            x += i;
        }
        sink = x;
    }
    
    if (__builtin_cpu_is("intel")) {
        /* Intel-specific cache configurations */
        volatile int y = 0;
        for (int i = 0; i < 1000; i++) {
            y -= i;
        }
        sink = y;
    }
    
    return 0;
}

/* { dg-final { scan-assembler "cache" } } */
/* { dg-final { scan-tree-dump "cpu" "optimized" } } */
