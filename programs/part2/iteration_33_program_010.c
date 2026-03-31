/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[64];
static int cpu_models[16];
static volatile int timing_results[8];

/* Early CPU initialization - runs before main */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
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
    cpu_features[11] = __builtin_cpu_supports("avx512vl");
    cpu_features[12] = __builtin_cpu_supports("fma");
    cpu_features[13] = __builtin_cpu_supports("aes");
    cpu_features[14] = __builtin_cpu_supports("pclmul");
    cpu_features[15] = __builtin_cpu_supports("rdrand");
    cpu_features[16] = __builtin_cpu_supports("rdseed");
    cpu_features[17] = __builtin_cpu_supports("sha");
    cpu_features[18] = __builtin_cpu_supports("xsave");
    cpu_features[19] = __builtin_cpu_supports("xsaveopt");
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");
    cpu_models[8] = __builtin_cpu_is("znver2");
    cpu_models[9] = __builtin_cpu_is("znver3");
}

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB array */
    if (array) {
        /* Access with 64-byte stride (typical cache line) */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            array[i] = i & 0xFF;
        }
        free((void*)array);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(2 * 1024 * 1024); /* 2MB array */
    if (array) {
        /* Access with 32-byte and 64-byte strides */
        for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
            array[i] = i & 0xFF;
        }
        for (int i = 64; i < 2 * 1024 * 1024; i += 64) {
            array[i] = (i >> 8) & 0xFF;
        }
        free((void*)array);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *array = malloc(4 * 1024 * 1024); /* 4MB array */
    if (array) {
        /* More complex access pattern */
        for (int i = 0; i < 4 * 1024 * 1024; i += 128) {
            array[i] = i & 0xFF;
            array[i + 64] = (i >> 8) & 0xFF;
        }
        free((void*)array);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(8 * 1024 * 1024); /* 8MB array */
    if (array) {
        /* Large stride access */
        for (int i = 0; i < 8 * 1024 * 1024; i += 256) {
            array[i] = i & 0xFF;
        }
        free((void*)array);
    }
}

/* Function to measure cache-sensitive timing */
static void measure_cache_timing(void) {
#define ARRAY_SIZE (4 * 1024 * 1024) /* 4MB - larger than typical L2 */
    volatile char *array = malloc(ARRAY_SIZE);
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i & 0xFF;
    }
    
    /* Time linear access (cache friendly) */
    uint64_t start, end;
    
    /* Use inline assembly for RDTSC to avoid library dependencies */
    __asm__ __volatile__ ("rdtsc" : "=a" (start) : : "rdx");
    start = (start & 0xFFFFFFFF) | ((uint64_t)end << 32);
    
    volatile char sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i += 64) { /* 64-byte cache line stride */
        sum += array[i];
    }
    
    __asm__ __volatile__ ("rdtsc" : "=a" (end) : : "rdx");
    end = (end & 0xFFFFFFFF) | ((uint64_t)end << 32);
    
    timing_results[0] = (int)(end - start);
    
    /* Time random access (cache unfriendly) */
    __asm__ __volatile__ ("rdtsc" : "=a" (start) : : "rdx");
    start = (start & 0xFFFFFFFF) | ((uint64_t)end << 32);
    
    for (int i = 0; i < 100000; i++) {
        int idx = (i * 97) % ARRAY_SIZE; /* Pseudo-random pattern */
        sum += array[idx];
    }
    
    __asm__ __volatile__ ("rdtsc" : "=a" (end) : : "rdx");
    end = (end & 0xFFFFFFFF) | ((uint64_t)end << 32);
    
    timing_results[1] = (int)(end - start);
    
    free((void*)array);
    
    /* Use the results to prevent dead code elimination */
    timing_results[2] = sum;
}

/* AVX2-specific test with pragma */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void avx2_cache_test(void) {
    /* Use AVX2 instructions that may trigger cache-specific optimizations */
    volatile __m256i *vec_array = malloc(1024 * sizeof(__m256i));
    if (vec_array) {
        __m256i vec = _mm256_set1_epi32(1);
        for (int i = 0; i < 1024; i++) {
            vec_array[i] = _mm256_add_epi32(vec_array[i], vec);
        }
        free((void*)vec_array);
    }
}
#pragma GCC pop_options

/* Main function with branching based on CPU features */
int main(void) {
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    
    /* Call AVX2 test if supported */
    if (cpu_features[9]) { /* avx2 */
        avx2_cache_test();
    }
    
    /* Measure cache timing */
    measure_cache_timing();
    
    /* Compute checksum from all CPU feature and model results */
    for (int i = 0; i < 20; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    
    for (int i = 0; i < 10; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    
    for (int i = 0; i < 3; i++) {
        checksum ^= timing_results[i];
    }
    
    /* Output checksum to prevent dead code elimination */
    printf("CPU detection checksum: %d\n", checksum);
    
    /* Additional feature queries to ensure driver processes more cases */
    if (__builtin_cpu_supports("popcnt")) checksum++;
    if (__builtin_cpu_supports("lzcnt")) checksum++;
    if (__builtin_cpu_supports("bmi")) checksum++;
    if (__builtin_cpu_supports("bmi2")) checksum++;
    if (__builtin_cpu_supports("rtm")) checksum++;
    if (__builtin_cpu_supports("hle")) checksum++;
    
    return checksum & 0xFF;
}
