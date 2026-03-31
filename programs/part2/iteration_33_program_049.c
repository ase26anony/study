/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */
/* This forces execution of CPUID interpretation for various cache descriptors */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global feature flags set by constructor */
static int cpu_features[32];
static int cpu_models[8];
static volatile int timing_results[4];

/* Early CPU initialization - runs before main */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
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
    cpu_features[12] = __builtin_cpu_supports("aes");
    cpu_features[13] = __builtin_cpu_supports("pclmul");
    cpu_features[14] = __builtin_cpu_supports("rdrand");
    cpu_features[15] = __builtin_cpu_supports("rdseed");
    cpu_features[16] = __builtin_cpu_supports("sha");
    cpu_features[17] = __builtin_cpu_supports("fma");
    cpu_features[18] = __builtin_cpu_supports("fma4");
    cpu_features[19] = __builtin_cpu_supports("xsave");
    cpu_features[20] = __builtin_cpu_supports("xsaveopt");
    cpu_features[21] = __builtin_cpu_supports("xsavec");
    cpu_features[22] = __builtin_cpu_supports("xgetbv");
    
    /* Query CPU models - each may have different cache descriptors */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("atom");
    cpu_models[3] = __builtin_cpu_is("core2");
    cpu_models[4] = __builtin_cpu_is("nehalem");
    cpu_models[5] = __builtin_cpu_is("sandybridge");
    cpu_models[6] = __builtin_cpu_is("haswell");
    cpu_models[7] = __builtin_cpu_is("skylake");
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = malloc(256 * 1024); /* 256KB - typical L2 for Core2 */
    if (buffer) {
        for (int i = 0; i < 256 * 1024; i += 64) { /* 64-byte lines */
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024); /* 1MB - typical L3 slice */
    if (buffer) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *buffer = malloc(2048 * 1024); /* 2MB */
    if (buffer) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(4096 * 1024); /* 4MB */
    if (buffer) {
        for (int i = 0; i < 4096 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        free((void*)buffer);
    }
}

/* Cache-sensitive timing test */
static void perform_cache_timing(void) {
#define ARRAY_SIZE (4 * 1024 * 1024) /* 4MB - larger than most L3 caches */
    static volatile char array[ARRAY_SIZE];
    uint64_t start, end;
    
    /* Time linear access with 64-byte stride */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < ARRAY_SIZE; i += 64) {
        array[i] = array[i] + 1;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[0] = (int)(end - start);
    
    /* Time linear access with 32-byte stride */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < ARRAY_SIZE; i += 32) {
        array[i] = array[i] + 1;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[1] = (int)(end - start);
    
    /* Time random access pattern */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < 100000; i++) {
        int idx = (i * 97) % ARRAY_SIZE; /* Pseudo-random pattern */
        array[idx] = array[idx] + 1;
    }
    end = __builtin_ia32_rdtsc();
    timing_results[2] = (int)(end - start);
}

/* Force vectorization with different ISA extensions */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    volatile int *buf = malloc(1024 * sizeof(int));
    if (buf) {
        for (int i = 0; i < 1024; i++) {
            buf[i] = i;
        }
        free((void*)buf);
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    volatile int *buf = malloc(2048 * sizeof(int));
    if (buf) {
        for (int i = 0; i < 2048; i++) {
            buf[i] = i;
        }
        free((void*)buf);
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
    if (cpu_features[7]) { /* If AVX2 is supported */
        avx512_cache_test();
    }
    
    /* Perform cache timing measurements */
    perform_cache_timing();
    
    /* Compute checksum from all results to prevent optimization */
    for (int i = 0; i < 23; i++) {
        checksum ^= cpu_features[i] << (i % 16);
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i] << (i % 16);
    }
    for (int i = 0; i < 4; i++) {
        checksum ^= timing_results[i];
    }
    
    /* Use checksum in a way that can't be optimized away */
    volatile int result = checksum;
    
    printf("CPU detection test completed. Checksum: %d\n", checksum);
    
    /* Additional feature queries to hit more driver paths */
    if (__builtin_cpu_supports("mmx")) checksum++;
    if (__builtin_cpu_supports("3dnow")) checksum++;
    if (__builtin_cpu_supports("popcnt")) checksum++;
    if (__builtin_cpu_supports("bmi")) checksum++;
    if (__builtin_cpu_supports("bmi2")) checksum++;
    if (__builtin_cpu_supports("lzcnt")) checksum++;
    if (__builtin_cpu_supports("f16c")) checksum++;
    if (__builtin_cpu_supports("movbe")) checksum++;
    
    return checksum & 0xFF;
}
