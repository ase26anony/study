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
static volatile int sink = 0;

/* Early CPU initialization with constructor attribute */
__attribute__((constructor))
static void init_cpu(void) {
    /* Force driver to initialize CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different paths */
    cpu_features[0] = __builtin_cpu_supports("mmx");
    cpu_features[1] = __builtin_cpu_supports("sse");
    cpu_features[2] = __builtin_cpu_supports("sse2");
    cpu_features[3] = __builtin_cpu_supports("sse3");
    cpu_features[4] = __builtin_cpu_supports("ssse3");
    cpu_features[5] = __builtin_cpu_supports("sse4.1");
    cpu_features[6] = __builtin_cpu_supports("sse4.2");
    cpu_features[7] = __builtin_cpu_supports("avx");
    cpu_features[8] = __builtin_cpu_supports("avx2");
    cpu_features[9] = __builtin_cpu_supports("avx512f");
    cpu_features[10] = __builtin_cpu_supports("fma");
    cpu_features[11] = __builtin_cpu_supports("aes");
    cpu_features[12] = __builtin_cpu_supports("pclmul");
    cpu_features[13] = __builtin_cpu_supports("rdrand");
    cpu_features[14] = __builtin_cpu_supports("rdseed");
    cpu_features[15] = __builtin_cpu_supports("sha");
    cpu_features[16] = __builtin_cpu_supports("xsave");
    cpu_features[17] = __builtin_cpu_supports("xsaveopt");
    cpu_features[18] = __builtin_cpu_supports("xsavec");
    cpu_features[19] = __builtin_cpu_supports("xsaves");
    
    /* Query CPU models to trigger different cache descriptor tables */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("atom");
    cpu_models[3] = __builtin_cpu_is("core2");
    cpu_models[4] = __builtin_cpu_is("nehalem");
    cpu_models[5] = __builtin_cpu_is("sandybridge");
    cpu_models[6] = __builtin_cpu_is("haswell");
    cpu_models[7] = __builtin_cpu_is("skylake");
}

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static int test_cache_core2(void) {
    volatile int sum = 0;
    char buffer[32768]; /* 32KB - typical L1 cache size */
    
    /* Access pattern that depends on cache line size */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    return sum;
}

__attribute__((target("arch=sandybridge")))
static int test_cache_sandybridge(void) {
    volatile int sum = 0;
    char buffer[262144]; /* 256KB - typical L2 cache size */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    return sum;
}

__attribute__((target("arch=haswell")))
static int test_cache_haswell(void) {
    volatile int sum = 0;
    char buffer[8388608]; /* 8MB - typical L3 cache size */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    return sum;
}

/* Function with AVX2 target to trigger feature-specific paths */
__attribute__((target("avx2")))
static void avx2_cache_test(void) {
    volatile int dummy = 0;
    alignas(32) float data[1024];
    
    #pragma GCC ivdep
    for (int i = 0; i < 1024; i++) {
        data[i] = i * 0.5f;
        dummy += (int)data[i];
    }
    sink = dummy;
}

/* SSE4.2 targeted function */
__attribute__((target("sse4.2")))
static void sse42_cache_test(void) {
    volatile int dummy = 0;
    alignas(16) int data[2048];
    
    for (int i = 0; i < 2048; i++) {
        data[i] = i;
        dummy ^= data[i];
    }
    sink = dummy;
}

/* Main cache timing test */
static uint64_t measure_cache_access(int stride, size_t size) {
    volatile char *buffer = (volatile char*)malloc(size);
    uint64_t start, end;
    
    if (!buffer) return 0;
    
    /* Initialize buffer */
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (char)(i & 0xFF);
    }
    
    /* Simple timing using rdtsc */
    start = __builtin_ia32_rdtsc();
    
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        sum += buffer[i];
    }
    
    end = __builtin_ia32_rdtsc();
    
    sink = sum; /* Prevent optimization */
    free((void*)buffer);
    
    return end - start;
}

int main(void) {
    uint64_t timing_results[4];
    int checksum = 0;
    
    /* Call target-specific functions */
    checksum += test_cache_core2();
    checksum += test_cache_sandybridge();
    checksum += test_cache_haswell();
    
    avx2_cache_test();
    sse42_cache_test();
    
    /* Measure with different strides to trigger cache size queries */
    timing_results[0] = measure_cache_access(32, 64 * 1024);   /* L1 sized */
    timing_results[1] = measure_cache_access(64, 64 * 1024);   /* L1 with 64B lines */
    timing_results[2] = measure_cache_access(64, 512 * 1024);  /* L2 sized */
    timing_results[3] = measure_cache_access(64, 8 * 1024 * 1024); /* L3 sized */
    
    /* Use timing results to create branches that might trigger cache queries */
    if (timing_results[1] * 2 < timing_results[3]) {
        checksum += 1; /* Likely cache hierarchy present */
    }
    
    if (timing_results[0] < timing_results[2] / 4) {
        checksum += 2; /* L1 faster than L2 */
    }
    
    /* Incorporate CPU feature flags into checksum */
    for (int i = 0; i < 20; i++) {
        checksum += cpu_features[i] ? (1 << (i % 16)) : 0;
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += cpu_models[i] ? (i + 1) : 0;
    }
    
    /* Additional feature queries in main to ensure driver usage */
    if (__builtin_cpu_supports("popcnt")) checksum += 1000;
    if (__builtin_cpu_supports("bmi")) checksum += 2000;
    if (__builtin_cpu_supports("bmi2")) checksum += 3000;
    if (__builtin_cpu_supports("lzcnt")) checksum += 4000;
    
    /* Final output to prevent dead code elimination */
    printf("CPU checksum: %d\n", checksum);
    printf("Timing ratios: %lu %lu %lu %lu\n", 
           timing_results[0], timing_results[1], 
           timing_results[2], timing_results[3]);
    
    return 0;
}

/* Additional pragmas to force driver consideration */
#pragma GCC target("arch=nehalem")
static void unused_nehalem_func(void) {
    volatile int x = 0;
    x = __builtin_ia32_rdtsc() & 1;
    sink = x;
}

#pragma GCC target("arch=skylake-avx512")
static void unused_skylake_func(void) {
    volatile int x = 0;
    x = __builtin_ia32_rdtsc() & 2;
    sink = x;
}
