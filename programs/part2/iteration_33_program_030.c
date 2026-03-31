/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int timing_results[4];

/* Early CPU initialization to force driver cache detection */
__attribute__((constructor(101)))
static void init_cpu_detection(void) {
    /* This forces the driver to execute CPUID and cache detection */
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

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024); /* 1MB > typical L2 */
    if (buffer) {
        /* Access with 64-byte stride (typical cache line) */
        for (size_t i = 0; i < 1024 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(2 * 1024 * 1024); /* 2MB */
    if (buffer) {
        /* Access with 32-byte stride */
        for (size_t i = 0; i < 2 * 1024 * 1024; i += 32) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *buffer = malloc(4 * 1024 * 1024); /* 4MB */
    if (buffer) {
        /* Random access pattern to stress cache */
        for (size_t i = 0; i < 10000; i++) {
            size_t idx = (i * 97) % (4 * 1024 * 1024);
            buffer[idx] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(8 * 1024 * 1024); /* 8MB */
    if (buffer) {
        /* Sequential access */
        for (size_t i = 0; i < 8 * 1024 * 1024; i++) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

/* AVX2 target to trigger AVX-related cache considerations */
#pragma GCC push_options
#pragma GCC target("avx2")
#include <immintrin.h>
static void avx2_cache_sensitive_op(void) {
    /* Use AVX2 operations that might be cache-sensitive */
    __m256i data = _mm256_set1_epi32(42);
    volatile __m256i *buffer = malloc(64 * sizeof(__m256i));
    if (buffer) {
        for (int i = 0; i < 64; i++) {
            buffer[i] = data;
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

/* Timing function using RDTSC */
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/* Cache size estimation through timing */
static void measure_cache_timing(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32MB */
    volatile char *buffer = malloc(max_size);
    
    if (!buffer) return;
    
    /* Warm up */
    for (size_t i = 0; i < max_size; i += 64) {
        buffer[i] = 0;
    }
    
    /* Measure different access patterns */
    uint64_t start, end;
    
    /* Linear access, 64-byte stride */
    start = rdtsc();
    for (size_t i = 0; i < max_size; i += 64) {
        buffer[i] = i & 0xFF;
    }
    end = rdtsc();
    timing_results[0] = (int)(end - start);
    
    /* Linear access, 32-byte stride */
    start = rdtsc();
    for (size_t i = 0; i < max_size; i += 32) {
        buffer[i] = i & 0xFF;
    }
    end = rdtsc();
    timing_results[1] = (int)(end - start);
    
    /* Random access */
    start = rdtsc();
    for (size_t i = 0; i < 100000; i++) {
        size_t idx = (i * 97) % max_size;
        buffer[idx] = i & 0xFF;
    }
    end = rdtsc();
    timing_results[2] = (int)(end - start);
    
    free((void*)buffer);
}

int main(void) {
    /* Execute all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    avx2_cache_sensitive_op();
    
    /* Measure cache timing */
    measure_cache_timing();
    
    /* Compute checksum from feature flags and timing to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 21; i++) {
        checksum = (checksum * 31) + cpu_features[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum = (checksum * 31) + cpu_models[i];
    }
    for (int i = 0; i < 4; i++) {
        checksum = (checksum * 31) + timing_results[i];
    }
    
    /* Output minimal result to satisfy compiler */
    printf("CPU detection test complete. Checksum: %d\n", checksum);
    
    /* Additional feature queries to ensure driver runs more detection */
    if (__builtin_cpu_supports("avx512vbmi")) checksum++;
    if (__builtin_cpu_supports("avx512vnni")) checksum++;
    if (__builtin_cpu_supports("avx512bitalg")) checksum++;
    if (__builtin_cpu_supports("avx512vpopcntdq")) checksum++;
    
    return checksum == 0 ? 0 : 1;
}

/* Additional constructor with higher priority to ensure early execution */
__attribute__((constructor(100)))
static void early_cpu_init(void) {
    /* Force early CPUID */
    volatile int x = 0;
    __builtin_cpu_init();
    /* Access to prevent dead code elimination */
    x = __builtin_cpu_supports("sse") ? 1 : 0;
    (void)x;
}
