/* { dg-do run { target i?86-*-* x86_64-*-* } } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink;

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
    cpu_features[11] = __builtin_cpu_supports("fma");
    cpu_features[12] = __builtin_cpu_supports("aes");
    cpu_features[13] = __builtin_cpu_supports("pclmul");
    cpu_features[14] = __builtin_cpu_supports("rdrand");
    cpu_features[15] = __builtin_cpu_supports("rdseed");
    cpu_features[16] = __builtin_cpu_supports("sha");
    cpu_features[17] = __builtin_cpu_supports("xsave");
    cpu_features[18] = __builtin_cpu_supports("xsaveopt");
    
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

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *array = malloc(256 * 1024); /* 256KB - typical L2 size */
    if (array) {
        for (int i = 0; i < 256 * 1024; i += 64) { /* 64-byte stride */
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB */
    if (array) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *array = malloc(2048 * 1024); /* 2MB */
    if (array) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(4096 * 1024); /* 4MB */
    if (array) {
        for (int i = 0; i < 4096 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

/* AVX2-optimized memory test to trigger feature-specific paths */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void avx2_cache_test(void) {
    volatile __m256i *array = malloc(1024 * 1024);
    if (array) {
        __m256i zero = _mm256_setzero_si256();
        for (int i = 0; i < (1024 * 1024) / sizeof(__m256i); i += 8) {
            _mm256_store_si256((__m256i*)&array[i], zero);
        }
        free((void*)array);
    }
}
#pragma GCC pop_options

/* SSE4.2-optimized memory test */
#pragma GCC push_options
#pragma GCC target("sse4.2")
#include <nmmintrin.h>
static void sse42_cache_test(void) {
    volatile __m128i *array = malloc(512 * 1024);
    if (array) {
        __m128i zero = _mm_setzero_si128();
        for (int i = 0; i < (512 * 1024) / sizeof(__m128i); i += 8) {
            _mm_store_si128((__m128i*)&array[i], zero);
        }
        free((void*)array);
    }
}
#pragma GCC pop_options

/* Cache size detection through timing */
static uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

static void timing_based_cache_test(void) {
    const size_t max_size = 8 * 1024 * 1024; /* 8MB */
    volatile char *array = malloc(max_size);
    
    if (!array) return;
    
    /* Initialize array */
    for (size_t i = 0; i < max_size; i++) {
        array[i] = (char)(i & 0xFF);
    }
    
    /* Test different access patterns that are cache-size sensitive */
    uint64_t times[4] = {0};
    
    /* Small stride (within cache line) */
    uint64_t start = rdtsc();
    for (size_t i = 0; i < max_size; i += 32) {
        sink = array[i];
    }
    times[0] = rdtsc() - start;
    
    /* Medium stride (multiple cache lines) */
    start = rdtsc();
    for (size_t i = 0; i < max_size; i += 128) {
        sink = array[i];
    }
    times[1] = rdtsc() - start;
    
    /* Large stride (likely cache miss) */
    start = rdtsc();
    for (size_t i = 0; i < max_size; i += 4096) {
        sink = array[i];
    }
    times[2] = rdtsc() - start;
    
    /* Random access pattern */
    start = rdtsc();
    for (size_t i = 0; i < 100000; i++) {
        size_t idx = (i * 97) % max_size; /* Simple pseudo-random */
        sink = array[idx];
    }
    times[3] = rdtsc() - start;
    
    /* Use timing results to prevent dead code elimination */
    if (times[0] > times[1]) {
        sink = 1;
    }
    
    free((void*)array);
}

int main(void) {
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    
    /* Call feature-specific tests */
    if (cpu_features[9]) { /* AVX2 */
        avx2_cache_test();
    }
    if (cpu_features[7]) { /* SSE4.2 */
        sse42_cache_test();
    }
    
    /* Perform timing-based cache test */
    timing_based_cache_test();
    
    /* Compute checksum from CPU feature results to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 19; i++) {
        checksum = (checksum << 1) | cpu_features[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum = (checksum << 1) | cpu_models[i];
    }
    
    printf("CPU checksum: %d\n", checksum);
    printf("Test completed - driver cache detection should have been triggered\n");
    
    return 0;
}
