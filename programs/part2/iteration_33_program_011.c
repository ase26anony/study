/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink; /* Prevent dead code elimination */

/* Early CPU initialization - forces driver cache detection */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* This triggers the driver's CPUID and cache detection logic */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to force driver 
       to examine different cache configurations */
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
    cpu_features[16] = __builtin_cpu_supports("xsaveopt");
    cpu_features[17] = __builtin_cpu_supports("xsavec");
    cpu_features[18] = __builtin_cpu_supports("xsaves");
    cpu_features[19] = __builtin_cpu_supports("mmx");
    cpu_features[20] = __builtin_cpu_supports("3dnow");
    cpu_features[21] = __builtin_cpu_supports("3dnowa");
    
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
    volatile char *buffer = malloc(1024 * 1024); /* 1MB */
    if (buffer) {
        /* Access with 64-byte stride (typical cache line) */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(2 * 1024 * 1024); /* 2MB */
    if (buffer) {
        /* Access with 32-byte stride */
        for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *buffer = malloc(8 * 1024 * 1024); /* 8MB */
    if (buffer) {
        /* Random access pattern to stress cache */
        for (int i = 0; i < 10000; i++) {
            int idx = (i * 997) % (8 * 1024 * 1024);
            sink = buffer[idx];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    /* Use AVX-512 if available */
    #pragma GCC target("avx512f")
    {
        volatile char *buffer = malloc(16 * 1024 * 1024); /* 16MB */
        if (buffer) {
            /* Large stride to bypass L1/L2 */
            for (int i = 0; i < 16 * 1024 * 1024; i += 4096) {
                sink = buffer[i];
            }
            free((void*)buffer);
        }
    }
}

/* Function to measure cache-sensitive timing */
static uint64_t measure_cache_access(int size_kb, int stride) {
    volatile char *buffer = malloc(size_kb * 1024);
    uint64_t start, end;
    
    if (!buffer) return 0;
    
    /* Simple timing using rdtsc */
    start = __builtin_ia32_rdtsc();
    
    for (int i = 0; i < size_kb * 1024; i += stride) {
        sink = buffer[i];
    }
    
    end = __builtin_ia32_rdtsc();
    free((void*)buffer);
    
    return end - start;
}

int main(void) {
    uint64_t timing_results[4];
    int checksum = 0;
    
    /* Call target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    
    /* Measure with different sizes and strides to trigger 
       cache size detection heuristics */
    timing_results[0] = measure_cache_access(8, 32);   /* L1-like */
    timing_results[1] = measure_cache_access(64, 64);  /* L1/L2 boundary */
    timing_results[2] = measure_cache_access(256, 64); /* L2-like */
    timing_results[3] = measure_cache_access(1024, 64); /* L3-like */
    
    /* Create checksum from feature flags and timing ratios */
    for (int i = 0; i < 22; i++) {
        checksum ^= cpu_features[i] << (i % 16);
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i] << (i % 16);
    }
    
    /* Use timing ratios to create branch diversity */
    if (timing_results[1] > timing_results[0] * 3) {
        checksum |= 0x10000; /* Likely L2 cache miss penalty */
    }
    if (timing_results[3] > timing_results[2] * 2) {
        checksum |= 0x20000; /* Likely L3 cache miss penalty */
    }
    
    /* Force compiler to consider all code paths */
    volatile int result = checksum;
    
    printf("CPU detection test completed. Checksum: %d\n", checksum);
    
    /* Additional feature queries in main to ensure driver runs */
    #pragma GCC target("avx2")
    if (__builtin_cpu_supports("avx2")) {
        result ^= 0x40000;
    }
    
    #pragma GCC target("aes")
    if (__builtin_cpu_supports("aes")) {
        result ^= 0x80000;
    }
    
    return result & 0xFF; /* Return non-zero to prevent optimization */
}
