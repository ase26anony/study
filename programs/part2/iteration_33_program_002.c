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

/* Early CPU initialization - mimics driver startup */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPUID and cache detection */
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
    cpu_features[10] = __builtin_cpu_supports("aes");
    cpu_features[11] = __builtin_cpu_supports("pclmul");
    cpu_features[12] = __builtin_cpu_supports("rdrand");
    cpu_features[13] = __builtin_cpu_supports("rdseed");
    cpu_features[14] = __builtin_cpu_supports("sha");
    cpu_features[15] = __builtin_cpu_supports("fma");
    cpu_features[16] = __builtin_cpu_supports("f16c");
    cpu_features[17] = __builtin_cpu_supports("bmi");
    cpu_features[18] = __builtin_cpu_supports("bmi2");
    cpu_features[19] = __builtin_cpu_supports("adx");
    cpu_features[20] = __builtin_cpu_supports("clflushopt");
    cpu_features[21] = __builtin_cpu_supports("xsave");
    cpu_features[22] = __builtin_cpu_supports("xsaveopt");
    
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

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    volatile char *array = malloc(8192 * 1024); /* 8MB */
    if (array) {
        for (int i = 0; i < 8192 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

/* Cache-sensitive memory access pattern */
static uint64_t cache_sensitive_access(int size_kb, int stride) {
    volatile char *array = malloc(size_kb * 1024);
    uint64_t start, end;
    
    if (!array) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size_kb * 1024; i++) {
        array[i] = (char)(i & 0xFF);
    }
    
    /* Simple timing using rdtsc */
    start = __builtin_ia32_rdtsc();
    
    /* Access pattern with given stride */
    for (int i = 0; i < size_kb * 1024; i += stride) {
        sink = array[i];
    }
    
    end = __builtin_ia32_rdtsc();
    free((void*)array);
    
    return end - start;
}

/* Test different cache line sizes */
static void test_cache_lines(void) {
    uint64_t time32, time64;
    
    /* Test with 32-byte stride (potential cache line) */
    time32 = cache_sensitive_access(64, 32);
    
    /* Test with 64-byte stride (typical cache line) */
    time64 = cache_sensitive_access(64, 64);
    
    /* Use results to prevent dead code elimination */
    if (time64 > time32) {
        sink = 1;
    } else {
        sink = 0;
    }
}

/* Main function with vectorized sections to trigger AVX paths */
int main(void) {
    uint64_t checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    
    /* Test cache line detection */
    test_cache_lines();
    
    /* Vectorized section with pragma to trigger AVX detection */
    #pragma GCC push_options
    #pragma GCC target("avx2")
    {
        volatile int vec[8] = {0};
        for (int i = 0; i < 8; i++) {
            vec[i] = i * i;
            sink = vec[i];
        }
    }
    #pragma GCC pop_options
    
    /* Another vectorized section for SSE */
    #pragma GCC push_options
    #pragma GCC target("sse4.2")
    {
        volatile float fvec[4] = {1.0f, 2.0f, 3.0f, 4.0f};
        for (int i = 0; i < 4; i++) {
            sink = (int)fvec[i];
        }
    }
    #pragma GCC pop_options
    
    /* Compute checksum from CPU feature flags */
    for (int i = 0; i < 23; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    
    for (int i = 0; i < 8; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    
    /* Output checksum to prevent optimization */
    printf("CPU checksum: %llu\n", (unsigned long long)checksum);
    
    /* Additional CPUID queries in main to ensure driver paths are taken */
    if (__builtin_cpu_supports("popcnt")) {
        sink = __builtin_popcount(checksum);
    }
    
    if (__builtin_cpu_supports("lzcnt")) {
        sink = __builtin_clz(checksum);
    }
    
    return 0;
}
