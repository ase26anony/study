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
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - forces driver cache detection */
__attribute__((constructor))
static void init_cpu(void) {
    /* This forces GCC's driver to execute CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different paths */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("fma");
    cpu_features[10] = __builtin_cpu_supports("aes");
    cpu_features[11] = __builtin_cpu_supports("pclmul");
    cpu_features[12] = __builtin_cpu_supports("rdrand");
    cpu_features[13] = __builtin_cpu_supports("rdseed");
    cpu_features[14] = __builtin_cpu_supports("sha");
    cpu_features[15] = __builtin_cpu_supports("xsave");
    
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
static void core2_cache_test(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB */
    for (int i = 0; i < 1024 * 1024; i += 64) { /* 64-byte stride */
        sink = array[i];
    }
    free((void*)array);
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(2 * 1024 * 1024); /* 2MB */
    for (int i = 0; i < 2 * 1024 * 1024; i += 32) { /* 32-byte stride */
        sink = array[i];
    }
    free((void*)array);
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *array = malloc(8 * 1024 * 1024); /* 8MB */
    for (int i = 0; i < 8 * 1024 * 1024; i += 128) { /* 128-byte stride */
        sink = array[i];
    }
    free((void*)array);
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(16 * 1024 * 1024); /* 16MB */
    for (int i = 0; i < 16 * 1024 * 1024; i += 256) { /* 256-byte stride */
        sink = array[i];
    }
    free((void*)array);
}

/* AVX2 vectorized memory access to trigger feature-specific cache logic */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void avx2_cache_test(void) {
    volatile __m256i *array = malloc(4 * 1024 * 1024);
    __m256i sum = _mm256_setzero_si256();
    
    for (int i = 0; i < (4 * 1024 * 1024) / 32; i++) {
        sum = _mm256_add_epi32(sum, array[i]);
    }
    
    sink = _mm256_extract_epi32(sum, 0);
    free((void*)array);
}
#pragma GCC pop_options

/* Cache size detection through timing */
static uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

static void measure_cache_effects(void) {
    const size_t size = 32 * 1024 * 1024; /* 32MB - larger than most L3 caches */
    volatile char *array = calloc(size, 1);
    uint64_t start, end;
    
    /* Sequential access - should hit in cache */
    start = rdtsc();
    for (size_t i = 0; i < size; i += 64) {
        sink = array[i];
    }
    end = rdtsc();
    uint64_t seq_time = end - start;
    
    /* Random access - more cache misses */
    start = rdtsc();
    for (size_t i = 0; i < 1000000; i++) {
        size_t idx = (i * 97) % size; /* Pseudo-random pattern */
        sink = array[idx];
    }
    end = rdtsc();
    uint64_t rand_time = end - start;
    
    /* Use timing ratio to branch - forces compiler to consider cache effects */
    if (rand_time > seq_time * 3) {
        sink = 1; /* Likely large cache */
    } else {
        sink = 0; /* Likely smaller cache */
    }
    
    free((void*)array);
}

int main(void) {
    int checksum = 0;
    
    /* Execute all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    
    /* Execute feature-specific tests if supported */
    if (cpu_features[7]) { /* AVX2 */
        avx2_cache_test();
    }
    
    /* Measure cache effects */
    measure_cache_effects();
    
    /* Compute checksum from CPU feature flags to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        checksum ^= cpu_features[i] << i;
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i] << (16 + i);
    }
    
    printf("CPU checksum: %d\n", checksum);
    printf("Test completed - driver cache detection should have been triggered\n");
    
    return 0;
}
