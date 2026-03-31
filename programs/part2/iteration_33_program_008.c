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

/* Early CPU initialization to force driver cache detection */
__attribute__((constructor))
static void init_cpu(void) {
    /* Force CPUID initialization */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different cache paths */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("avx512vl");
    cpu_features[10] = __builtin_cpu_supports("avx512bw");
    cpu_features[11] = __builtin_cpu_supports("avx512dq");
    cpu_features[12] = __builtin_cpu_supports("fma");
    cpu_features[13] = __builtin_cpu_supports("aes");
    cpu_features[14] = __builtin_cpu_supports("pclmul");
    cpu_features[15] = __builtin_cpu_supports("rdrand");
    cpu_features[16] = __builtin_cpu_supports("rdseed");
    cpu_features[17] = __builtin_cpu_supports("sha");
    cpu_features[18] = __builtin_cpu_supports("xsave");
    cpu_features[19] = __builtin_cpu_supports("xsaveopt");
    cpu_features[20] = __builtin_cpu_supports("xsavec");
    cpu_features[21] = __builtin_cpu_supports("xsaves");
    
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
    volatile char *buffer = malloc(1024 * 1024);
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
    volatile char *buffer = malloc(2 * 1024 * 1024);
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
    volatile char *buffer = malloc(4 * 1024 * 1024);
    if (buffer) {
        /* Random access pattern to stress cache */
        for (int i = 0; i < 10000; i++) {
            int idx = (i * 997) % (4 * 1024 * 1024);
            sink = buffer[idx];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(8 * 1024 * 1024);
    if (buffer) {
        /* Sequential access */
        for (int i = 0; i < 8 * 1024 * 1024; i += 128) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    volatile char *buffer = malloc(16 * 1024 * 1024);
    if (buffer) {
        /* Large stride access */
        for (int i = 0; i < 16 * 1024 * 1024; i += 256) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

/* Function with pragma to trigger AVX2 path */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_sensitive(void) {
    /* Use AVX2 if available */
    if (__builtin_cpu_supports("avx2")) {
        volatile int *arr = malloc(1024 * sizeof(int));
        if (arr) {
            for (int i = 0; i < 1024; i++) {
                arr[i] = i;
            }
            sink = arr[511];
            free((void*)arr);
        }
    }
}
#pragma GCC pop_options

/* Timing function using RDTSC */
static inline uint64_t rdtsc(void) {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/* Cache size detection through timing */
static void detect_cache_through_timing(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32 MB */
    volatile char *buffer = malloc(max_size);
    uint64_t times[5];
    
    if (!buffer) return;
    
    /* Warm up cache */
    for (size_t i = 0; i < max_size; i += 64) {
        buffer[i] = 1;
    }
    
    /* Time different access patterns */
    uint64_t start, end;
    
    /* Test 1: Sequential access, 64-byte stride */
    start = rdtsc();
    for (size_t i = 0; i < max_size; i += 64) {
        sink = buffer[i];
    }
    end = rdtsc();
    times[0] = end - start;
    
    /* Test 2: Sequential access, 32-byte stride */
    start = rdtsc();
    for (size_t i = 0; i < max_size; i += 32) {
        sink = buffer[i];
    }
    end = rdtsc();
    times[1] = end - start;
    
    /* Test 3: Random access within first 1MB (likely L2/L1 cache) */
    start = rdtsc();
    for (int i = 0; i < 10000; i++) {
        int idx = (i * 997) % (1024 * 1024);
        sink = buffer[idx];
    }
    end = rdtsc();
    times[2] = end - start;
    
    /* Test 4: Random access across entire range */
    start = rdtsc();
    for (int i = 0; i < 10000; i++) {
        int idx = (i * 997) % max_size;
        sink = buffer[idx];
    }
    end = rdtsc();
    times[3] = end - start;
    
    /* Use timing differences to branch (simulating cache-sensitive code) */
    if (times[3] > times[2] * 3) {
        /* Large difference suggests cache hierarchy is working */
        cpu_features[22] = 1;
    }
    
    free((void*)buffer);
}

int main(void) {
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    avx2_cache_sensitive();
    
    /* Perform cache timing detection */
    detect_cache_through_timing();
    
    /* Compute checksum from all feature flags to prevent optimization */
    for (int i = 0; i < 23; i++) {
        checksum ^= cpu_features[i] << (i % 8);
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i] << (i % 8);
    }
    
    /* Force compiler to consider all paths by using checksum */
    if (checksum & 1) {
        printf("CPU features detected\n");
    } else {
        printf("Basic CPU detection\n");
    }
    
    /* Additional feature queries that might trigger cache logic */
    if (__builtin_cpu_supports("mmx")) checksum++;
    if (__builtin_cpu_supports("3dnow")) checksum++;
    if (__builtin_cpu_supports("popcnt")) checksum++;
    if (__builtin_cpu_supports("movbe")) checksum++;
    if (__builtin_cpu_supports("f16c")) checksum++;
    if (__builtin_cpu_supports("bmi")) checksum++;
    if (__builtin_cpu_supports("bmi2")) checksum++;
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
