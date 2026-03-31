/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[256];
static volatile int sink = 0;

/* Early CPU initialization - runs before main */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force CPUID initialization */
    __builtin_cpu_init();
    
    /* Query a wide range of CPU features to trigger cache detection */
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
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("ivybridge");
    cpu_models[6] = __builtin_cpu_is("haswell");
    cpu_models[7] = __builtin_cpu_is("broadwell");
    cpu_models[8] = __builtin_cpu_is("skylake");
    cpu_models[9] = __builtin_cpu_is("cannonlake");
    cpu_models[10] = __builtin_cpu_is("icelake");
    cpu_models[11] = __builtin_cpu_is("tigerlake");
    cpu_models[12] = __builtin_cpu_is("alderlake");
    cpu_models[13] = __builtin_cpu_is("znver1");
    cpu_models[14] = __builtin_cpu_is("znver2");
    cpu_models[15] = __builtin_cpu_is("znver3");
    cpu_models[16] = __builtin_cpu_is("znver4");
}

/* Cache-sensitive memory access patterns */
static void cache_test_32(void) {
    /* Test with 32-byte stride (typical for some cache lines) */
    volatile char *buffer = malloc(1024 * 1024 * 4); /* 4MB buffer */
    if (!buffer) return;
    
    for (int i = 0; i < 1024 * 1024 * 4; i += 32) {
        sink += buffer[i];
    }
    
    free((void*)buffer);
}

static void cache_test_64(void) {
    /* Test with 64-byte stride (typical cache line size) */
    volatile char *buffer = malloc(1024 * 1024 * 8); /* 8MB buffer */
    if (!buffer) return;
    
    for (int i = 0; i < 1024 * 1024 * 8; i += 64) {
        sink += buffer[i];
    }
    
    free((void*)buffer);
}

/* Target-specific functions to force different microarchitecture considerations */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    cache_test_32();
    cache_test_64();
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    cache_test_32();
    cache_test_64();
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    cache_test_32();
    cache_test_64();
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    cache_test_32();
    cache_test_64();
}

__attribute__((target("arch=znver2")))
static void zen2_cache_test(void) {
    cache_test_32();
    cache_test_64();
}

/* Vectorized operations with different ISA extensions */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    /* AVX2 operations that might trigger cache considerations */
    volatile int *data = malloc(1024 * 1024 * sizeof(int));
    if (!data) return;
    
    for (int i = 0; i < 1024 * 1024; i += 8) {
        /* Simulate some vectorizable work */
        sink += data[i] + data[i+1] + data[i+2] + data[i+3] +
                data[i+4] + data[i+5] + data[i+6] + data[i+7];
    }
    
    free((void*)data);
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    /* AVX-512 operations */
    volatile int *data = malloc(1024 * 1024 * sizeof(int));
    if (!data) return;
    
    for (int i = 0; i < 1024 * 1024; i += 16) {
        /* Simulate wider vector work */
        for (int j = 0; j < 16; j++) {
            sink += data[i + j];
        }
    }
    
    free((void*)data);
}
#pragma GCC pop_options

/* Timing-based cache size estimation */
static void timing_based_cache_detection(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32MB */
    volatile char *buffer = malloc(max_size);
    if (!buffer) return;
    
    /* Initialize buffer */
    for (size_t i = 0; i < max_size; i++) {
        buffer[i] = (char)(i & 0xFF);
    }
    
    /* Test different access patterns */
    uint64_t time1 = 0, time2 = 0;
    
    /* Small stride (likely in L1) */
    for (size_t i = 0; i < 8192; i += 64) {
        sink += buffer[i];
    }
    
    /* Large stride (likely beyond L3) */
    for (size_t i = 0; i < max_size; i += 4096) {
        sink += buffer[i];
    }
    
    free((void*)buffer);
}

int main(void) {
    int result = 0;
    
    /* Execute all cache tests */
    cache_test_32();
    cache_test_64();
    
    /* Execute target-specific tests */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen2_cache_test();
    
    /* Execute vectorized tests if supported */
    if (cpu_features[6]) { /* AVX */
        avx2_cache_test();
    }
    
    if (cpu_features[8]) { /* AVX-512 */
        avx512_cache_test();
    }
    
    /* Timing-based detection */
    timing_based_cache_detection();
    
    /* Compute checksum from CPU feature results to prevent optimization */
    for (int i = 0; i < 20; i++) {
        result += cpu_features[i];
    }
    
    for (int i = 0; i < 17; i++) {
        result += cpu_models[i];
    }
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        printf("Unexpected result\n");
    }
    
    printf("CPU feature checksum: %d\n", result);
    printf("Test completed - driver cache detection should have been triggered\n");
    
    return 0;
}
