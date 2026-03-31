/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic" } */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[64];
static int cpu_models[16];
static volatile int sink;

/* Early CPU initialization - runs before main */
static void __attribute__((constructor)) init_cpu(void) {
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
    cpu_features[12] = __builtin_cpu_supports("avx512bw");
    cpu_features[13] = __builtin_cpu_supports("avx512dq");
    cpu_features[14] = __builtin_cpu_supports("fma");
    cpu_features[15] = __builtin_cpu_supports("aes");
    cpu_features[16] = __builtin_cpu_supports("pclmul");
    cpu_features[17] = __builtin_cpu_supports("popcnt");
    cpu_features[18] = __builtin_cpu_supports("movbe");
    cpu_features[19] = __builtin_cpu_supports("rdrnd");
    cpu_features[20] = __builtin_cpu_supports("fsgsbase");
    cpu_features[21] = __builtin_cpu_supports("bmi");
    cpu_features[22] = __builtin_cpu_supports("bmi2");
    cpu_features[23] = __builtin_cpu_supports("lzcnt");
    cpu_features[24] = __builtin_cpu_supports("f16c");
    cpu_features[25] = __builtin_cpu_supports("xsave");
    cpu_features[26] = __builtin_cpu_supports("xsaveopt");
    
    /* Query CPU models to trigger different cache descriptor paths */
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
}

/* Target-specific functions to force driver to consider different architectures */
static void __attribute__((target("arch=core2"))) 
core2_cache_test(void) {
    volatile char *buffer = malloc(256 * 1024); /* 256KB - typical L2 for Core2 */
    if (buffer) {
        for (int i = 0; i < 256 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

static void __attribute__((target("arch=sandybridge"))) 
sandybridge_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024); /* 1MB - typical L3 for Sandy Bridge */
    if (buffer) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

static void __attribute__((target("arch=skylake"))) 
skylake_cache_test(void) {
    volatile char *buffer = malloc(2048 * 1024); /* 2MB - typical L3 for Skylake */
    if (buffer) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

static void __attribute__((target("arch=znver2"))) 
zen2_cache_test(void) {
    volatile char *buffer = malloc(4096 * 1024); /* 4MB - typical L3 for Zen 2 */
    if (buffer) {
        for (int i = 0; i < 4096 * 1024; i += 64) {
            buffer[i] = i & 0xFF;
        }
        sink = buffer[0];
        free((void*)buffer);
    }
}

/* Cache-sensitive memory access patterns */
static uint64_t measure_cache_access(int stride, int size_kb) {
    uint64_t start, end;
    volatile char *buffer = malloc(size_kb * 1024);
    uint64_t sum = 0;
    
    if (!buffer) return 0;
    
    /* Initialize buffer */
    for (int i = 0; i < size_kb * 1024; i++) {
        buffer[i] = i & 0xFF;
    }
    
    /* Simple timing using rdtsc */
    start = __builtin_ia32_rdtsc();
    
    /* Access pattern with given stride */
    for (int i = 0; i < size_kb * 1024; i += stride) {
        sum += buffer[i];
    }
    
    end = __builtin_ia32_rdtsc();
    
    sink = sum;
    free((void*)buffer);
    
    return end - start;
}

/* Test different cache line sizes */
static void test_cache_lines(void) {
    uint64_t time32, time64, time128;
    
    /* Test with 32-byte stride (potential cache line) */
    time32 = measure_cache_access(32, 1024);
    
    /* Test with 64-byte stride (typical cache line) */
    time64 = measure_cache_access(64, 1024);
    
    /* Test with 128-byte stride (two cache lines) */
    time128 = measure_cache_access(128, 1024);
    
    /* Use results to prevent optimization */
    if (time64 < time128) {
        sink = 1;
    }
}

/* Vectorized loop with pragma to trigger AVX detection */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    volatile int *buffer = malloc(8192 * sizeof(int));
    if (buffer) {
        for (int i = 0; i < 8192; i++) {
            buffer[i] = i;
        }
        
        /* Simple computation that might use AVX2 */
        int sum = 0;
        for (int i = 0; i < 8192; i += 8) {
            sum += buffer[i];
        }
        sink = sum;
        free((void*)buffer);
    }
}
#pragma GCC pop_options

/* Another vectorized test with SSE */
#pragma GCC push_options
#pragma GCC target("sse4.2")
static void sse42_cache_test(void) {
    volatile float *buffer = malloc(16384 * sizeof(float));
    if (buffer) {
        for (int i = 0; i < 16384; i++) {
            buffer[i] = i * 0.1f;
        }
        
        float sum = 0.0f;
        for (int i = 0; i < 16384; i += 4) {
            sum += buffer[i];
        }
        sink = (int)sum;
        free((void*)buffer);
    }
}
#pragma GCC pop_options

int main(void) {
    int result = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    skylake_cache_test();
    zen2_cache_test();
    
    /* Test cache-sensitive access patterns */
    test_cache_lines();
    
    /* Call vectorized functions */
    avx2_cache_test();
    sse42_cache_test();
    
    /* Compute checksum from CPU feature flags to prevent dead code elimination */
    for (int i = 0; i < 27; i++) {
        result ^= cpu_features[i] << (i % 16);
    }
    
    for (int i = 0; i < 16; i++) {
        result ^= cpu_models[i] << (i % 16);
    }
    
    /* Additional CPU feature queries in main to ensure driver runs */
    if (__builtin_cpu_supports("sse")) {
        result |= 0x1000;
    }
    if (__builtin_cpu_supports("avx")) {
        result |= 0x2000;
    }
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x4000;
    }
    
    /* Check specific cache-related CPUID leaves by querying features
       that are associated with particular cache configurations */
    if (__builtin_cpu_supports("aes")) {
        /* AES-NI often appears with specific cache sizes */
        result |= 0x8000;
    }
    
    if (__builtin_cpu_supports("pclmul")) {
        /* PCLMUL often appears with specific cache sizes */
        result |= 0x10000;
    }
    
    /* Force evaluation of CPU model strings */
    const char *models[] = {
        "intel", "amd", "core2", "nehalem", "sandybridge",
        "ivybridge", "haswell", "broadwell", "skylake"
    };
    
    for (int i = 0; i < 9; i++) {
        if (__builtin_cpu_is(models[i])) {
            result ^= (i + 1) * 0x1111;
        }
    }
    
    printf("CPU test result: 0x%x\n", result);
    
    return 0;
}
