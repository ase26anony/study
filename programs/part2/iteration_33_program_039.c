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
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - forces driver cache detection */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* This triggers the driver's CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query various CPU features to force driver through different paths */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("aes");
    cpu_features[10] = __builtin_cpu_supports("pclmul");
    cpu_features[11] = __builtin_cpu_supports("rdrand");
    cpu_features[12] = __builtin_cpu_supports("rdseed");
    cpu_features[13] = __builtin_cpu_supports("sha");
    cpu_features[14] = __builtin_cpu_supports("fma");
    cpu_features[15] = __builtin_cpu_supports("f16c");
    
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
static void test_core2_cache(void) {
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
static void test_sandybridge_cache(void) {
    volatile char *buffer = malloc(2 * 1024 * 1024); /* 2MB */
    if (buffer) {
        /* Access with 32-byte and 64-byte strides */
        for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
            sink = buffer[i];
        }
        for (int i = 0; i < 2 * 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void test_haswell_cache(void) {
    volatile char *buffer = malloc(8 * 1024 * 1024); /* 8MB */
    if (buffer) {
        /* Random access pattern to stress cache */
        unsigned int seed = 12345;
        for (int i = 0; i < 10000; i++) {
            int idx = (rand_r(&seed) % (8 * 1024 * 1024)) & ~63; /* Cache line aligned */
            sink = buffer[idx];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=znver1")))
static void test_zen_cache(void) {
    volatile char *buffer = malloc(16 * 1024 * 1024); /* 16MB */
    if (buffer) {
        /* Large stride to potentially miss all caches */
        for (int i = 0; i < 16 * 1024 * 1024; i += 1024 * 1024) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

/* Function with pragmas to trigger different optimization paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    /* Use AVX2 instructions that might be cache-sensitive */
    volatile int *data = malloc(256 * 1024); /* 256KB */
    if (data) {
        for (int i = 0; i < 256 * 1024 / sizeof(int); i += 8) {
            /* Simulate vectorized access pattern */
            sink = data[i];
        }
        free((void*)data);
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    /* Larger accesses for AVX512 */
    volatile int *data = malloc(512 * 1024); /* 512KB */
    if (data) {
        for (int i = 0; i < 512 * 1024 / sizeof(int); i += 16) {
            sink = data[i];
        }
        free((void*)data);
    }
}
#pragma GCC pop_options

/* Cache size estimation via timing */
static uint64_t rdtsc(void) {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

static void measure_cache_effects(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32MB */
    volatile char *buffer = malloc(max_size);
    
    if (!buffer) return;
    
    /* Warm up */
    for (size_t i = 0; i < max_size; i += 4096) {
        sink = buffer[i];
    }
    
    /* Measure different access patterns */
    uint64_t times[4] = {0};
    
    /* Sequential access - should hit caches */
    uint64_t start = rdtsc();
    for (size_t i = 0; i < max_size; i += 64) {
        sink = buffer[i];
    }
    times[0] = rdtsc() - start;
    
    /* Random access within L1/L2 sized regions */
    start = rdtsc();
    unsigned int seed = 42;
    for (int i = 0; i < 100000; i++) {
        size_t idx = (rand_r(&seed) % (256 * 1024)) & ~63; /* L2-ish size */
        sink = buffer[idx];
    }
    times[1] = rdtsc() - start;
    
    /* Large stride access */
    start = rdtsc();
    for (size_t i = 0; i < max_size; i += 1024 * 1024) {
        sink = buffer[i];
    }
    times[2] = rdtsc() - start;
    
    free((void*)buffer);
    
    /* Use timing results to prevent dead code elimination */
    sink = (int)(times[0] ^ times[1] ^ times[2]);
}

int main(void) {
    int checksum = 0;
    
    /* Call all target-specific functions */
    test_core2_cache();
    test_sandybridge_cache();
    test_haswell_cache();
    test_zen_cache();
    
    avx2_cache_test();
    avx512_cache_test();
    
    /* Perform cache timing measurements */
    measure_cache_effects();
    
    /* Compute checksum from CPU feature flags to prevent optimization */
    for (int i = 0; i < 16; i++) {
        checksum ^= cpu_features[i] << i;
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i] << (16 + i);
    }
    
    /* Output something to ensure execution */
    printf("CPU checksum: %d\n", checksum);
    
    return 0;
}
