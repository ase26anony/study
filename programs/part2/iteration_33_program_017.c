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
static volatile int timing_results[4];

/* Early CPU initialization - runs before main */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different paths */
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
    
    /* Query CPU models to trigger different cache descriptor tables */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");
}

/* Target-specific functions to force driver to consider different microarchitectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024); /* 1MB buffer */
    if (buffer) {
        /* Access with 64-byte stride (typical cache line) */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(2 * 1024 * 1024); /* 2MB buffer */
    if (buffer) {
        /* Access with 32-byte stride */
        for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *buffer = malloc(4 * 1024 * 1024); /* 4MB buffer */
    if (buffer) {
        /* Mixed stride pattern */
        for (int i = 0; i < 4 * 1024 * 1024; i += 128) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(8 * 1024 * 1024); /* 8MB buffer */
    if (buffer) {
        /* Larger stride to potentially miss L2 */
        for (int i = 0; i < 8 * 1024 * 1024; i += 256) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

/* Function to perform cache-sensitive timing */
static void measure_cache_timing(void) {
#define ARRAY_SIZE (4 * 1024 * 1024) /* 4MB - larger than typical L2 */
    static volatile char array[ARRAY_SIZE];
    uint64_t start, end;
    
    /* Time linear access (should hit caches well) */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < ARRAY_SIZE; i += 64) {
        array[i] = i & 0xFF;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[0] = (int)(end - start);
    
    /* Time random access (more cache misses) */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < ARRAY_SIZE; i += 97) { /* Prime stride */
        array[i % ARRAY_SIZE] = i & 0xFF;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[1] = (int)(end - start);
    
    /* Time with 32-byte stride */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < ARRAY_SIZE; i += 32) {
        array[i] = i & 0xFF;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[2] = (int)(end - start);
    
    /* Time with 128-byte stride */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < ARRAY_SIZE; i += 128) {
        array[i] = i & 0xFF;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[3] = (int)(end - start);
}

/* AVX2-optimized function to trigger vectorization and cache considerations */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void avx2_cache_test(void) {
    volatile __m256i *buffer = malloc(1024 * sizeof(__m256i));
    if (buffer) {
        __m256i pattern = _mm256_set1_epi32(0x12345678);
        for (int i = 0; i < 1024; i++) {
            buffer[i] = pattern;
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

/* AES-specific test to trigger crypto extensions */
#pragma GCC push_options
#pragma GCC target("aes,pclmul")
#include <wmmintrin.h>
static void aes_cache_test(void) {
    volatile __m128i *buffer = malloc(1024 * sizeof(__m128i));
    if (buffer) {
        __m128i key = _mm_set1_epi32(0x12345678);
        __m128i data = _mm_set1_epi32(0x87654321);
        for (int i = 0; i < 1024; i++) {
            buffer[i] = _mm_aesenc_si128(data, key);
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

int main(void) {
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    
    /* Call vectorized functions */
    avx2_cache_test();
    aes_cache_test();
    
    /* Perform cache timing measurements */
    measure_cache_timing();
    
    /* Compute checksum from all results to prevent dead code elimination */
    for (int i = 0; i < 21; i++) {
        checksum ^= cpu_features[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i];
    }
    for (int i = 0; i < 4; i++) {
        checksum ^= timing_results[i];
    }
    
    /* Use checksum in a way that can't be optimized away */
    volatile int result = checksum;
    
    printf("CPU detection and cache test completed. Checksum: %d\n", result);
    
    /* Additional feature queries to ensure driver paths are taken */
    if (__builtin_cpu_supports("sse4.2")) {
        printf("SSE4.2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
    }
    
    return 0;
}
